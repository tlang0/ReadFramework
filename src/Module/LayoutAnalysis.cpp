/*******************************************************************************************************
 ReadFramework is the basis for modules developed at CVL/TU Wien for the EU project READ. 
  
 Copyright (C) 2016 Markus Diem <diem@caa.tuwien.ac.at>
 Copyright (C) 2016 Stefan Fiel <fiel@caa.tuwien.ac.at>
 Copyright (C) 2016 Florian Kleber <kleber@caa.tuwien.ac.at>

 This file is part of ReadFramework.

 ReadFramework is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ReadFramework is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The READ project  has  received  funding  from  the European  Union’s  Horizon  2020  
 research  and innovation programme under grant agreement No 674943
 
 related links:
 [1] http://www.caa.tuwien.ac.at/cvl/
 [2] https://transkribus.eu/Transkribus/
 [3] https://github.com/TUWien/
 [4] http://nomacs.org
 *******************************************************************************************************/

#include "LayoutAnalysis.h"

#include "Image.h"

#include "Utils.h"
#include "GraphCut.h"
#include "SuperPixelScaleSpace.h"
#include "TextLineSegmentation.h"
#include "Elements.h"
#include "ElementsHelper.h"
#include "PageParser.h"
#include "LineTrace.h"

#pragma warning(push, 0)	// no warnings from includes
 // Qt Includes
#pragma warning(pop)

namespace rdf {


// LayoutAnalysisConfig --------------------------------------------------------------------
LayoutAnalysisConfig::LayoutAnalysisConfig() : ModuleConfig("Layout Analysis Module") {
}

QString LayoutAnalysisConfig::toString() const {
	return ModuleConfig::toString();
}

void LayoutAnalysisConfig::setRemoveWeakTextLiens(bool remove) {
	mRemoveWeakTextLines = remove;
}

bool LayoutAnalysisConfig::removeWeakTextLines() const {
	return mRemoveWeakTextLines;
}

void LayoutAnalysisConfig::setMinSuperPixelsPerBlock(int minPx) {
	mMinSuperPixelsPerBlock = minPx;
}

int LayoutAnalysisConfig::minSuperixelsPerBlock() const {
	return ModuleConfig::checkParam(mMinSuperPixelsPerBlock, 0, INT_MAX, "minSuperPixelsPerBlock");
}

void LayoutAnalysisConfig::setLocalBlockOrientation(bool lor) {
	mLocalBlockOrientation = lor;
}

bool LayoutAnalysisConfig::localBlockOrientation() const {
	return mLocalBlockOrientation;
}

void LayoutAnalysisConfig::setComputeSeparators(bool cs) {
	mComputeSeparators = cs;
}

bool LayoutAnalysisConfig::computeSeparators() const {
	return mComputeSeparators;
}

void LayoutAnalysisConfig::load(const QSettings & settings) {

	mMinSuperPixelsPerBlock	= settings.value("minSuperPixelsPerBlock", minSuperixelsPerBlock()).toInt();
	mRemoveWeakTextLines	= settings.value("removeWeakTextLines", removeWeakTextLines()).toBool();
	mLocalBlockOrientation	= settings.value("localBlockOrientation", localBlockOrientation()).toBool();
	mComputeSeparators		= settings.value("computeSeparators", computeSeparators()).toBool();
}

void LayoutAnalysisConfig::save(QSettings & settings) const {
	
	settings.setValue("minSuperPixelsPerBlock", minSuperixelsPerBlock());
	settings.setValue("removeWeakTextLines", removeWeakTextLines());
	settings.setValue("localBlockOrientation", localBlockOrientation());
	settings.setValue("computeSeparators", computeSeparators());
}

// LayoutAnalysis --------------------------------------------------------------------
LayoutAnalysis::LayoutAnalysis(const cv::Mat& img) {

	mImg = img;

	// initialize scale factory
	ScaleFactory::instance().init(img.size());

	mConfig = QSharedPointer<LayoutAnalysisConfig>::create();
	mConfig->loadSettings();
}

bool LayoutAnalysis::isEmpty() const {
	return mImg.empty();
}

bool LayoutAnalysis::compute() {

	if (!checkInput())
		return false;

	Timer dt;

	cv::Mat img = mImg;

	if (img.rows > 2000) {

		// if you stumble upon this line:
		// microfilm images are binary with heavey noise
		// a small median filter fixes this issue...
		cv::medianBlur(img, img, 3);
	}

	qDebug() << "scale factor dpi: " << ScaleFactory::scaleFactorDpi();

	img = ScaleFactory::scaled(img);
	mImg = img;

	// find super pixels
	//ScaleSpaceSuperPixel<GridSuperPixel> spM(mImg);
	GridSuperPixel spM(img);

	if (!spM.compute()) {
		mWarning << "could not compute super pixels!";
		return false;
	}
	
	// TODO: automatically find text blocks

	// create an 'all-in' text block
	mTextBlockSet = createTextBlocks();

	if (mTextBlockSet.isEmpty()) {
		Rect r(Vector2D(), img.size());
		mTextBlockSet << Polygon::fromRect(r);
	}
	
	PixelSet pixels = spM.pixelSet();
	
	// scale back to original coordinates
	mTextBlockSet.setPixels(pixels);

	mStopLines = createStopLines();

	Timer dtTl;

	if (!config()->localBlockOrientation()) {
		if (!computeLocalStats(pixels))
			return false;
	}

	// compute text lines for each text block
	for (QSharedPointer<TextBlock> tb : mTextBlockSet.textBlocks()) {

		PixelSet sp = tb->pixelSet();

		if (sp.isEmpty()) {
			qInfo() << *tb << "is empty...";
			continue;
		}

		if (config()->localBlockOrientation()) {

			if (!computeLocalStats(sp))
				return false;
		}

		//// find tab stops
		//rdf::TabStopAnalysis tabStops(sp);
		//if (!tabStops.compute())
		//	qWarning() << "could not compute text block segmentation!";

		// find text lines
		QVector<QSharedPointer<TextLineSet> > textLines;
		if (sp.size() > config()->minSuperixelsPerBlock()) {

			rdf::TextLineSegmentation tlM(sp);
			tlM.addSeparatorLines(mStopLines);

			if (!tlM.compute()) {
				qWarning() << "could not compute text line segmentation!";
				return false;
			}

			// save text lines
			textLines = tlM.textLineSets();
		}

		// paragraph is a single textline
		if (textLines.empty()) {
			textLines << QSharedPointer<TextLineSet>(new TextLineSet(sp.pixels()));
		}

		tb->setTextLines(textLines);
	}
	
	mInfo << "Textlines computed in" << dtTl;

	// scale back to original coordinates
	ScaleFactory::scaleInv(mTextBlockSet);

	for (Line& l : mStopLines)
		l.scale(1.0 / ScaleFactory::scaleFactor());

	// clean-up
	if (config()->removeWeakTextLines()) {
		mTextBlockSet.removeWeakTextLines();
	}

	mInfo << "computed in" << dt;

	return true;
}

QSharedPointer<LayoutAnalysisConfig> LayoutAnalysis::config() const {
	return qSharedPointerDynamicCast<LayoutAnalysisConfig>(mConfig);
}

cv::Mat LayoutAnalysis::draw(const cv::Mat & img, const QColor& col) const {

	QImage qImg = Image::mat2QImage(img, true);
	
	QPainter p(&qImg);
	p.setPen(ColorManager::blue());

	for (auto l : mStopLines) {
		l.setThickness(3);
		l.draw(p);
	}

	for (auto tb : mTextBlockSet.textBlocks()) {

		QPen pen(col);
		pen.setCosmetic(true);
		p.setPen(pen);

		QVector<PixelSet> s = tb->pixelSet().splitScales();
		for (int idx = s.size()-1; idx >= 0; idx--) {
			
			if (!col.isValid())
				p.setPen(ColorManager::randColor());
			p.setOpacity(0.5);
			s[idx].draw(p, PixelSet::DrawFlags() | PixelSet::draw_pixels, Pixel::DrawFlags() | Pixel::draw_stats | Pixel::draw_ellipse);
			//qDebug() << "scale" << idx << ":" << *s[idx];
		}
				
		p.setOpacity(1.0);
		
		QPen tp(ColorManager::pink());
		tp.setWidth(5);
		p.setPen(tp);

		tb->draw(p, TextBlock::draw_text_lines);
	}
	
	return Image::qImage2Mat(qImg);
}

QString LayoutAnalysis::toString() const {
	return Module::toString();
}

void LayoutAnalysis::setRootRegion(const QSharedPointer<RootRegion>& region) {

	mRoot = region;
}

TextBlockSet LayoutAnalysis::textBlockSet() const {
	return mTextBlockSet;
}

QVector<SeparatorRegion> LayoutAnalysis::stopLines() const {
	
	QVector<SeparatorRegion> sps;
	for (auto l : mStopLines) {
		sps << SeparatorRegion::fromLine(l);
	}
	
	return sps;
}

PixelSet LayoutAnalysis::pixels() const {
	
	PixelSet set;
	for (auto tb : mTextBlockSet.textBlocks())
		set.append(tb->pixelSet().pixels());
	
	return set;
}

bool LayoutAnalysis::checkInput() const {

	return !isEmpty();
}

TextBlockSet LayoutAnalysis::createTextBlocks() const {

	if (mRoot) {
		
		// get (potential) text regions - from XML
		QVector<QSharedPointer<Region> > textRegions = RegionManager::filter<Region>(mRoot, Region::type_text_region);
		textRegions << RegionManager::filter<Region>(mRoot, Region::type_table_cell);

		TextBlockSet tbs(textRegions);
		tbs.scale(ScaleFactory::scaleFactor());

		return tbs;
	}

	return TextBlockSet();
}

QVector<Line> LayoutAnalysis::createStopLines() const {

	QVector<Line> stopLines;

	if (mRoot) {

		auto separators = RegionManager::filter<SeparatorRegion>(mRoot, Region::type_separator);

		for (auto s : separators) {
			Line sl = s->line();
			sl.scale(ScaleFactory::scaleFactor());
			stopLines << sl;
		}
	}

	if (stopLines.empty() &&  config()->computeSeparators()) {
		
		LineTraceLSD lt(mImg);
		lt.config()->setScale(1.0);

		if (!lt.compute()) {
			qWarning() << "could not compute separators...";
		}

		stopLines << lt.separatorLines();
	}

	return stopLines;
}

bool LayoutAnalysis::computeLocalStats(PixelSet & pixels) const {

	// find local orientation per pixel
	rdf::LocalOrientation lo(pixels);
	if (!lo.compute()) {
		qWarning() << "could not compute local orientation";
		return false;
	}

	// smooth orientation
	rdf::GraphCutOrientation pse(pixels);

	if (!pse.compute()) {
		qWarning() << "could not smooth orientation";
		return false;
	}

	// smooth line spacing
	rdf::GraphCutLineSpacing pls(pixels);

	if (!pls.compute()) {
		qWarning() << "could not smooth line spacing";
		return false;
	}

	return true;
}


}