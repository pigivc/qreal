#pragma once

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QMultiHash>

#include "textManagerInterface.h"
#include "view/editorView.h"
#include "toolPluginInterface/systemEventsInterface.h"

namespace qReal {

class TextManager : public TextManagerInterface
{
public:
	TextManager(SystemEventsInterface *systemEvents, MainWindow *mainWindow);

	/// Reads code source file and create new QScintillaTextEdit associated with this file (rather with filepath)
	bool openFile(QString const &filePath);

	/// Remove all info about filePath (including QScintillaTextEdit associated with it)
	bool closeFile(QString const &filePath);

	void changeFilePath(QString const &from, QString const &to);

	/// Binds diagram with another code source file.
	bool bindCode(EditorView* diagram,  QString const &filePath);

	bool unbindCode(QString const &filePath);
	bool unbindCode(gui::QScintillaTextEdit *code);
	gui::QScintillaTextEdit *code(QString const &filePath);
	QList<gui::QScintillaTextEdit *> code(EditorView *diagram);
	bool contains(QString const &filePath);
	bool removeDiagram(EditorView *diagram);
	EditorView *diagram(gui::QScintillaTextEdit *code);
	QString path(gui::QScintillaTextEdit *code);
	bool isDefaultPath(QString const &path);
	bool isModified(QString const &path);
	bool isModifiedEver(QString const &path);
	void addExtension(QString const &ext, QString const &description);
	QString extDescription(QString const &ext);

	/// Opens new tab with text editor and shows a text in it
	/// @param fileInfo A filepath to file with text
	void showInTextEditor(QFileInfo const &fileInfo);

	/// Saves text from tab to another or same file
	/// @param saveAs Defines what to do: save to the same file or in another
	bool saveText(bool saveAs);

private slots:
	void setModified(gui::QScintillaTextEdit *code);

private:
	QMap<QString, gui::QScintillaTextEdit *> mText;
	QMap<gui::QScintillaTextEdit *, QString> mPath;

	/// If default path - true.
	QMap<QString, bool> mPathType;

	/// First - was modified once and more (with save), second - was modified recently and not saved
	QMap<QString, QPair<bool, bool> > mModified;

	/// mCodeTabManager - Map that keeps pairs of opened tabs and their code areas.
	QMultiHash<EditorView*, QString> mDiagramCodeManager;

	/// Contains info about file extensions (extension, description of extension). Need for save dialog.
	QMap<QString, QString> mExtensions;
	MainWindow *mMainWindow;
	SystemEventsInterface *mSystemEvents;
};

}
