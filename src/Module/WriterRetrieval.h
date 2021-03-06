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
#include "WriterDatabase.h"


#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QVector>
#include <opencv2/imgproc.hpp>
#include <QSettings>
#pragma warning(pop)

#ifndef DllCoreExport
#ifdef DLL_CORE_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

#pragma warning(disable: 4251)	// dll interface

// Qt defines


namespace rdf {
	class WriterRetrievalConfig;

	class DllCoreExport WriterRetrieval : public Module {
	public:
		WriterRetrieval(cv::Mat img);

		bool isEmpty() const override;
		bool compute() override;
		QSharedPointer<WriterRetrievalConfig> config() const;
		cv::Mat getFeature();

		void setXmlPath(std::string xmlPath);

		cv::Mat draw(const cv::Mat& img) const;
		QString toString() const override;

	private:

		bool checkInput() const override;
		cv::Mat mImg;
		cv::Mat mFeature;

		cv::Mat mDesc = cv::Mat();
		QVector<cv::KeyPoint> mKeyPoints;

		QString mXmlPath = "";
		
		
	};

	class DllCoreExport WriterRetrievalConfig : public ModuleConfig {
	public:
		WriterRetrievalConfig();

		virtual QString toString() const override;
		bool isEmpty();
		WriterVocabulary vocabulary();

	protected:
		void load(const QSettings& settings) override;
		void save(QSettings& settings) const override;


	private:
		QString debugName();
		QString mDebugName = "Writer Retrieval Config";
		WriterVocabulary mVoc;
		QString mSettingsVocPath;
		QString mFeatureDir = "sift";
		QString mEvalFile = "";
	};

	class DllCoreExport WriterImage {

	public:
		WriterImage();

		void setImage(cv::Mat img);
		void calculateFeatures();
		void saveFeatures(QString filePath);
		void loadFeatures(QString filePath);


		void setKeyPoints(QVector<cv::KeyPoint> kp);
		QVector<cv::KeyPoint> keyPoints() const;
		void setDescriptors(cv::Mat desc);
		cv::Mat descriptors() const;
		
		void filterKeyPoints(int minSize, int maxSize);
		void filterKeyPointsPoly(QVector<QPolygonF> polys);

	private:
		QString debugName();

		cv::Mat mImg;
		cv::Mat mDescriptors;
		QVector<cv::KeyPoint> mKeyPoints;

	};
	

}