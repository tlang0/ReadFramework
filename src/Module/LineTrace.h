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

#pragma once

#include "BaseModule.h"
#include "Shapes.h"

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QVector>
#pragma warning(pop)

#pragma warning (disable: 4251)	// inlined Qt functions in dll interface

#ifndef DllModuleExport
#ifdef DLL_MODULE_EXPORT
#define DllModuleExport Q_DECL_EXPORT
#else
#define DllModuleExport Q_DECL_IMPORT
#endif
#endif

// Qt defines

namespace rdf {



class DllModuleExport LineTraceConfig : public ModuleConfig {

public:
	LineTraceConfig();

	float maxSlopeRotat() const;
	void setMaxSlopeRotat(float s);

	float maxLenDiff() const;
	void setMaxLenDiff(float l);

	float maxAspectRatio() const;
	void setMaxAspectRatio(float a);

	int minWidth() const;
	void setMinWidth(int w);

	int maxLen() const;
	void setMaxLen(int l);

	int minArea() const;
	void setMinArea(int a);

	int rippleLen() const;
	void setRippleLen(int r);

	float rippleArea() const;
	void setRippleArea(float a);

	float maxGap() const;
	void setMaxGap(float g);

	float maxSlopeDiff() const;
	void setMaxSlopeDiff(float s);

	float maxAngleDiff() const;
	void setMaxAngleDiff(float a);

	int minLenSecondRun() const;
	void setMinLenSecondRun(int r);

	float maxDistExtern() const;
	void setMaxDistExtern(float d);

	float maxAngleDiffExtern() const;
	void setMaxAngleDiffExtern(float a);


	QString toString() const override;

private:
	void load(const QSettings& settings) override;
	void save(QSettings& settings) const override;

	float mMaxSlopeRotat = 10.0f;	//filter parameter: maximal difference of line orientation compared to the result of the Rotation module (default: 5 deg)
	float mMaxLenDiff = 1.5f;		//filter parameter: maximal difference in length between two successive runlengths (default: 1.5)
	float mMaxAspectRatio = 0.3f;	//filter parameter: maximal aspect ratio of a line (default: 0.3f)
	int mMinWidth = 20;			//filter parameter: minimal width a line in pixel (default: 30)
	int mMaxLen = 20;				//filter parameter: maximal length of a line in pixel (default: 20)
	int mMinArea = 20;				//filter parameter: minimum area in pixel (default: 40)
	int mRippleLen = 200;			//filter parameter: ripple len of a line in pixel (default: 200)
	float mRippleArea = 0.2f;		//filter parameter: ripple area of a line (default: 0.2f)
	float mMaxGap = 100;			//filter parameter: maximal gap between two lines in pixel (default: 250)
	float mMaxSlopeDiff = 2.0f;		//filter parameter: maximal slope difference between two lines in degree (default: 3)
	float mMaxAngleDiff = 2.0f;		//filter parameter: maximal angle difference between two compared and the inserted line (default: 20)
	int mMinLenSecondRun = 60;    //min len to filter after merge lines; was 60

								  //filter Lines parameter (compared to given line vector, see std::vector<DkLineExt> filterLines(std::vector<DkLineExt> &externLines))
	float mMaxDistExtern = 10.0f;		//maximal Distance of the external line end points compared to a given line (default: 5 pixel)
	float mMaxAngleDiffExtern = 20.0f / 180.0f * (float)CV_PI;;	//maximal Angle Difference of the external line compared to a given line (default: 20 deg)
};

/// <summary>
/// Detects Lines in a binary image. The result is a binary image containing all line elements (pixel accurate) as well as all lines as vectors.
///	It is also possible to filter lines according a specified angle.
/// </summary>
/// <seealso cref="Module" />
	class DllModuleExport LineTrace : public Module {

public:
	LineTrace(const cv::Mat& img, const cv::Mat& mask = cv::Mat());

	bool isEmpty() const override;
	virtual bool compute() override;
	QVector<rdf::Line> filterLineAngle(const QVector<rdf::Line>& lines, float angle, float angleDiff) const;
	QVector<rdf::Line> getHLines() const;
	QVector<rdf::Line> getVLines() const;
	QVector<rdf::Line> getLines() const;
	void setAngle(double angle = std::numeric_limits<double>::infinity());
	void resetAngle();

	QSharedPointer<LineTraceConfig> config() const;

	/**
	* Returns a vector with all calculated and merged lines.
	* @return The calculated and merged lines.
	**/
	//QVector<DkLineExt> getLines();
	cv::Mat lineImage() const;
	cv::Mat generatedLineImage() const;
	static void generateLineImage(const QVector<rdf::Line>& hline, const QVector<rdf::Line>& vline, cv::Mat& img, cv::Scalar hCol = cv::Scalar(255), cv::Scalar vCol = cv::Scalar(255));
	//void setMinLenSecondRun(int len);
	//void setMaxAspectRatio(float ratio);
	virtual QString toString() const override;

protected:
	bool checkInput() const override;

	cv::Mat mSrcImg;									//the input image  either 3 channel or 1 channel [0 255]
	cv::Mat mLineImg;									//the line image [0 255]
	cv::Mat mMask;										//the mask image [0 255]

	QVector<rdf::Line> hLines;
	QVector<rdf::Line> vLines;


private:

	double mAngle = std::numeric_limits<double>::infinity();		//filter parameter: angle of the snippet determined by the skew estimation (default: 0.0f)

	float mLineProb;
	float mLineDistProb;

	//void load(const qsettings& settings) override;
	//void save(qsettings& settings) const override;
	cv::Mat hDSCC(const cv::Mat& bwImg) const;
	void filter(cv::Mat& hDSCCImg, cv::Mat& vDSCCImg);
	QVector<rdf::Line> mergeLines(QVector<rdf::Line>& lines);
	void filterLines();
	void drawGapLines(cv::Mat& img, QVector<rdf::Line> lines);
};

};