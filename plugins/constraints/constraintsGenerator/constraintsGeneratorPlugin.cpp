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

#include "constraintsGeneratorPlugin.h"

#include <QtCore/QProcess>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include <qrkernel/settingsManager.h>

using namespace constraints::generator;
using namespace qReal;

ConstraintsGeneratorPlugin::ConstraintsGeneratorPlugin()
{
	mAppTranslator.load(":/constraintsGenerator_" + QLocale::system().name());
	QApplication::installTranslator(&mAppTranslator);
}

ConstraintsGeneratorPlugin::~ConstraintsGeneratorPlugin()
{
}

void ConstraintsGeneratorPlugin::init(const PluginConfigurator &configurator)
{
	mMainWindowInterface = &configurator.mainWindowInterpretersInterface();
	mLogicalModel = &configurator.logicalModelApi();
	mGenerator.init(configurator.logicalModelApi(), *configurator.mainWindowInterpretersInterface().errorReporter());
}

QList<ActionInfo> ConstraintsGeneratorPlugin::actions()
{
	auto const generateAction = new QAction(tr("Generate constraints"), nullptr);
	connect(generateAction, &QAction::triggered, this, &ConstraintsGeneratorPlugin::generate);

	ActionInfo generateActionInfo(generateAction, "interpreters", "tools");

	return {generateActionInfo};
}

void ConstraintsGeneratorPlugin::generate()
{
	for (const qReal::Id &metamodel : mLogicalModel->logicalRepoApi().elementsByType("MetamodelConstraints")) {
		if (!mLogicalModel->logicalRepoApi().isLogicalElement(metamodel)) {
			continue;
		}

		mGenerator.generate(metamodel);

		const QString constraintModelFullName =  mGenerator.constraintModelFullName();
		const QString constraintModelName = mGenerator.constraintConstraintsModelName();
		const QString constraintNormalizerModelName = mGenerator.constraintNormalizedConstraintsModelName();
		const QString constraintModelId = mGenerator.constraintModelId();

		const QPair<QString, QString> constraintModelNames = QPair<QString, QString>(constraintModelName
				, constraintNormalizerModelName);

		if (!mMainWindowInterface->errorReporter()->wereErrors()) {
			if (QMessageBox::question(mMainWindowInterface->windowWidget()
					, tr("loading.."), QString(tr("Do you want to load generated constraints?")),
					QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
			{
				return;
			}

			loadNewEditor(constraintModelFullName
					, constraintModelNames
					, constraintModelId
					, SettingsManager::value("pathToQmake", "").toString()
					, SettingsManager::value("pathToMake", "").toString()
					, SettingsManager::value("pluginExtension", "").toString()
					, SettingsManager::value("prefix", "").toString()
					, mGenerator.buildConfiguration());
		}
	}
}

void ConstraintsGeneratorPlugin::loadNewEditor(
		const QString &directoryName
		, const QPair<QString, QString> &pluginsNames
		, const QString &pluginId
		, const QString &commandFirst
		, const QString &commandSecond
		, const QString &extension
		, const QString &prefix
		, const QString &buildConfiguration
		)
{
	const int progressBarWidth = 240;
	const int progressBarHeight = 20;

	const QString pluginName = pluginsNames.first;
	const QString normalizerPluginName = pluginsNames.second;

	if ((commandFirst == "") || (commandSecond == "") || (extension == "")) {
		QMessageBox::warning(mMainWindowInterface->windowWidget(), tr("error"), tr("please, fill compiler settings"));
		return;
	}

	QProgressBar * const progress = new QProgressBar(mMainWindowInterface->windowWidget());
	progress->show();

	QApplication::processEvents();

	const QRect screenRect = qApp->desktop()->availableGeometry();
	progress->move(screenRect.width() / 2 - progressBarWidth / 2, screenRect.height() / 2 - progressBarHeight / 2);
	progress->setFixedWidth(progressBarWidth);
	progress->setFixedHeight(progressBarHeight);
	progress->setRange(0, 100);
	progress->setValue(5);

	if (!mMainWindowInterface->unloadConstraintsPlugin(pluginName + "." + extension, pluginId)) {
		QMessageBox::warning(mMainWindowInterface->windowWidget(), tr("error"), tr("cannot unload plugin"));
		deleteGeneratedFiles(directoryName, normalizerPluginName);
		progress->close();
		delete progress;

		return;
	}

	progress->setValue(20);

	QProcess builder;
	builder.setWorkingDirectory(directoryName);
	builder.start(commandFirst, {"CONFIG+=" + buildConfiguration});

	if ((builder.waitForFinished()) && (builder.exitCode() == 0)) {
		progress->setValue(60);
		builder.start(commandSecond);

		if (builder.waitForFinished() && (builder.exitCode() == 0)) {
			progress->setValue(80);
			const QString buildConfigurationString = (buildConfiguration == "debug") ? "-d" : "";

			if (mMainWindowInterface->loadConstraintsPlugin(prefix + pluginName
					+ buildConfigurationString + "." + extension)) {
				progress->setValue(100);
			}
		}
	}

	if (progress->value() != 100) {
		QMessageBox::warning(mMainWindowInterface->windowWidget(), tr("error")
				, tr("cannot load new constraints plugin"));
		deleteGeneratedFiles(directoryName, normalizerPluginName);
	}

	progress->setValue(100);
	progress->close();
	delete progress;
}

void ConstraintsGeneratorPlugin::deleteGeneratedFiles(
		const QString &directoryName
		, const QString &fileBaseName)
{
	QFile filePro(directoryName + "/" + fileBaseName + ".pro");
	filePro.remove();
}