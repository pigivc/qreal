/* Copyright 2007-2015 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include <QtWidgets/QDockWidget>

namespace utils {

/// A container for some widget that bahaves like QDockWidget when it docked into the QMainWidow instance
/// and like separate QDialog when it is in the floatible state.
class SmartDock : public QDockWidget
{
	Q_OBJECT

public:
	/// Represents a state of this dock: it is docked into window or floats as QDialog.
	enum class Mode
	{
		/// Inner widget is embedded into this instance, widget is docked to some panel.
		Docked
		/// Inner widget is embedded into separate QDialog, floats and acts like regular window.
		, Floats
	};

	/// @param objectName A name that will be given to this dock like QObject instance and will
	/// be used for storing window size in SettingsManager.
	/// @param parent Host for this dock widget. If nullptr then dock widget will determine
	/// main window itself by enumerating QApplication::topLevelWidgets.
	SmartDock(const QString &objectName, QWidget *innerWidget, QMainWindow *parent = nullptr);
	~SmartDock();

	/// Embeds inner widget into this dock widget instance, docks it to some panel.
	void switchToDocked();

	/// Embeds inner widget into separate QDialog, makes it floating and acting like regular window.
	void switchToFloating();

private slots:
	/// Overrides default behaviour to float in QDialog shape.
	void checkFloating();

private:
	QMainWindow *findMainWindow() const;

	bool event(QEvent *event) override;

	void initDock();
	void initDialog();

	bool isAnimating();

	QMainWindow *mMainWindow;  // Doesn`t take ownerhsip
	QWidget *mInnerWidget;  // Doesn`t take ownerhsip
	QDialog *mDialog;  // Takes ownership
	Mode mCurrentMode;
	bool mDragged = false;
};

}
