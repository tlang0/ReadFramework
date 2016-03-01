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

#pragma warning(push, 0)	// no warnings from includes
#include <QObject>
#pragma warning(pop)

#include "opencv2/core/core.hpp"

#pragma warning (disable: 4251)	// inlined Qt functions in dll interface

#ifndef DllModuleExport
#ifdef DLL_MODULE_EXPORT
#define DllModuleExport Q_DECL_EXPORT
#else
#define DllModuleExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QSettings;

namespace rdf {

#define mDebug		qDebug().noquote() << debugName()
#define mWarning	qWarning().noquote() << debugName()
#define mInfo		qInfo().noquote() << debugName()


/// <summary>
/// This is the base class for all modules.
/// It provides all functions which are implemented by the modules.
/// </summary>
	class DllModuleExport Module {
	
public:

	/// <summary>
	/// Default constructor
	/// Initializes a new instance of the <see cref="Module"/> class.
	/// </summary>
	Module();

	friend DllModuleExport QDataStream& operator<<(QDataStream& s, const Module& m);
	friend DllModuleExport QDebug operator<< (QDebug d, const Module &m);


	 /// <summary>
	 /// Returns true if the module was initialized with the default constructor.
	 /// Note, if empty is true, nothing can be computed.
	 /// </summary>
	 /// <returns>Returns true if the module was initialized.</returns>
	virtual bool isEmpty() const = 0;

	 /// <summary>
	 /// Returns the module's name.
	 /// </summary>
	 /// <returns>The module's name.</returns>
	virtual QString name() const;

	 /// <summary>
	 /// Converts the module's parameters and results to a string.
	 /// </summary>
	 /// <returns>The string containing all parameters and results of the module.</returns>
	virtual QString toString() const;

	 /// <summary>
	 /// Runs the algorithm implemented by the module.
	 /// </summary>
	 /// <returns>True on success.</returns>
	virtual bool compute() = 0;

	/// <summary>
	/// Loads the settings.
	/// </summary>
	void loadSettings();

	/// <summary>
	/// Saves the settings.
	/// </summary>
	void saveSettings();

protected:
	QString mModuleName;						/**< the module's name.**/

	virtual bool checkInput() const = 0;		/**< checks if all input images are in the specified format.**/

	virtual void load(const QSettings& settings);
	virtual void save(QSettings& settings) const;
	QString debugName() const;

};

};
