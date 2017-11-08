/*
 *   Copyright 2013, Jorge Martinez Vargas ( jormarv5@fiv.upv.es )
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * 
 *      Author: Jorge Martinez Vargas ( jormarv5@fiv.upv.es )
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QCheckBox>
#include <QtCore/QVector>
#include <QtGui/QKeyEvent>
#include <QtGui/QComboBox>
#include <QToolBar>

#include "helpwindow.h"
#include "aboutwindow.h"
#include "glviewport.h"
#include "document.h"
#include "settingswindow.h"

// Definition of modes
#define LAYOUT_MODE 1
#define LINE_MODE 2

/* Stablished number of vertexs for the regions.
    Note that this value is used when adding regions but does not limit in any way the shape of regions that can be
    loaded from an extern XML file */
#define NUM_VERT 4

using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    /* GUI attributes */

    HelpWindow *help_window;
    AboutWindow *about_window;
    SettingsWindow *settings_window;

    QWidget *file_widget;
    QWidget *combo_widget;
    QWidget *pos_widget;
    QWidget *name_widget;

    QToolBar *toolbar;
    QHBoxLayout *file_layout;
    QHBoxLayout *combo_layout;
    QHBoxLayout *pos_layout;
    QHBoxLayout *name_layout;

    QMenu *file_menu;
    QAction *open_list_action;
    QAction *open_file_action;
    QAction *save_action;
    QAction *save_as_action;
    // El help menu està provisional
    QMenu *help_menu;
    QAction *help_action;
    QAction *about_action;
    QMenu *options_menu;
    QAction *settings_action;
    QMenu *mode_menu;
    QAction *linemode_action;
    QAction *layoutmode_action;

    QLineEdit *img_name;
    QLineEdit *file_number;
    QLabel *file_total;
    QLineEdit *pos_display;
    QComboBox *label_combobox;
    QLineEdit *id_selected_item;
    QAction *next_action;
    QAction *previous_action;

    /* Atributes */

    QVector<QString> *filelist;
    int current_file;
    Document *data;
    GLViewport *viewport;
    bool autosave;
    bool overwrite;
    int mode;

    /* Private methods */
    void createToolBar();
    void createMenuBar();
    void connectSignalsToSlots();
    void loadDocument();
    void saveDocument();
    void updateImgCount();
    void keyPressEvent(QKeyEvent *event);
    void switchPoints(Point &l, Point &r);
    void sortLayoutPoints(Point p[NUM_VERT]);

    void closeEvent(QCloseEvent *event);

public:
    MainWindow(QWidget *parent = 0);
    MainWindow(char* filename, bool single, QWidget *parent = 0);
    ~MainWindow();

    QString nextLabel(QString currentLabel);
    QString getSelectedLabel();

public slots:
    void modeLayout();
    void modeLines();
    void showHelp();
    void showAbout();
    void showSettings();
    void loadFile(char* filename = 0, bool single = false);
    void nextImage();
    void previousImage();
    void saveData();
    void saveDataAs();
    void updateLabelCombobox(QString label);
    void updatePosition(int x, int y);
    void updateID(QString id);
    void updateSettings(QMap<QString,QVariant> &settings);
    void loadSingleFile();
};

#endif // MAINWINDOW_H
