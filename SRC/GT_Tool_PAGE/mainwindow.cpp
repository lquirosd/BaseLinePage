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

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QTextEdit>

#include <iostream>
#include <sstream>

#include "mainwindow.h"
#include "document.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    viewport = new GLViewport(this);
  //viewport = new GLViewport(0,0, 500, 1000);
    viewport->makeCurrent();
    viewport->setFocus();
    filelist = 0;
    autosave = false;
    overwrite = true;
    mode = LAYOUT_MODE;
    data=0;
    setCentralWidget(viewport);
    createMenuBar();
    createToolBar();
    connectSignalsToSlots();
}

/* Editar per a que reba més coses, incialitzar estructura on se guarda la info de labels */
MainWindow::MainWindow(char* filename, bool single, QWidget *parent)
    : QMainWindow(parent)
{
    viewport = new GLViewport(this);
    viewport->makeCurrent();
    viewport->setFocus();
    filelist = 0;
    autosave = false;
    overwrite = true;
    mode = LAYOUT_MODE;
    data=0;
    setCentralWidget(viewport);
    createMenuBar();
    createToolBar();
    connectSignalsToSlots();
    if(filename)
        loadFile(filename,single);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createToolBar(){
    file_widget = new QWidget;
    file_layout = new QHBoxLayout;
    file_number = new QLineEdit("-");
    file_total = new QLabel("/ -");
    id_selected_item = new QLineEdit("-");
    id_selected_item->setReadOnly(true);
    file_number->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    id_selected_item->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    file_total->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    file_widget->setLayout(file_layout);
    file_layout->addWidget(file_number);
    file_layout->addWidget(file_total);

    pos_display = new QLineEdit("x: -, y: -");
    pos_display->setReadOnly(true);
    pos_display->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    pos_layout = new QHBoxLayout;
    pos_layout->addWidget(pos_display);
    pos_widget = new QWidget;
    pos_widget->setLayout(pos_layout);
    label_combobox = new QComboBox;
    label_combobox->addItem(T1);
    label_combobox->addItem(T2);
    label_combobox->addItem(T3);
    label_combobox->addItem(T4);
    label_combobox->addItem(T5);
    label_combobox->addItem(T6);
    label_combobox->addItem(T7);
    label_combobox->addItem(T8);
    label_combobox->addItem(T9);
    label_combobox->addItem(T10);
    label_combobox->addItem(T11);
    label_combobox->addItem(T12);
    label_combobox->addItem(T13);
    label_combobox->addItem(T14);
    label_combobox->addItem(T15);
    label_combobox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    combo_layout = new QHBoxLayout;
    combo_layout->addWidget(label_combobox);
    combo_layout->addWidget(id_selected_item);
    combo_widget = new QWidget;
    combo_widget->setLayout(combo_layout);
    img_name = new QLineEdit("No file loaded");
    img_name->setReadOnly(true);
    img_name->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    name_layout = new QHBoxLayout;
    name_layout->addWidget(img_name);
    name_widget = new QWidget;
    name_widget->setLayout(name_layout);
    next_action = new QAction(QIcon::fromTheme("go-next"),"",this);
    previous_action = new QAction(QIcon::fromTheme("go-previous"),"",this);

    toolbar = new QToolBar(this);
    toolbar->addWidget(file_widget);
    toolbar->addAction(previous_action);
    toolbar->addAction(next_action);
    toolbar->addSeparator();
    toolbar->addWidget(combo_widget);
    toolbar->addSeparator();
    toolbar->addWidget(pos_widget);
    toolbar->addSeparator();
    toolbar->addWidget(name_widget);

    addToolBar(Qt::TopToolBarArea,toolbar);

}

void MainWindow::createMenuBar(){
    file_menu = menuBar()->addMenu("File");
    open_file_action = new QAction("Open xml file",this);
    open_list_action = new QAction("Open list file",this);
    save_action = new QAction("Save (Ctrl+S)",this);
    save_as_action = new QAction("Save as (Ctrl+Shift+S)",this);
    file_menu->addAction(open_file_action);
    file_menu->addAction(open_list_action);
    file_menu->addAction(save_action);
    file_menu->addAction(save_as_action);
    mode_menu = menuBar()->addMenu("Mode");
    layoutmode_action = new QAction("Region mode (F1)",this);
    linemode_action = new QAction("Line mode (F2)",this);
    mode_menu->addAction(layoutmode_action);
    mode_menu->addAction(linemode_action);
    options_menu = menuBar()->addMenu("Options");
    settings_action = new QAction("Settings",this);
    options_menu->addAction(settings_action);
    help_menu = menuBar()->addMenu("Help");
    help_action = new QAction("Help",this);
    about_action = new QAction("About",this);
    help_menu->addAction(help_action);
    help_menu->addAction(about_action);

}

void MainWindow::connectSignalsToSlots(){
    connect(open_list_action,SIGNAL(triggered()),this,SLOT(loadFile()));
    connect(open_file_action,SIGNAL(triggered()),this,SLOT(loadSingleFile()));
    connect(help_action,SIGNAL(triggered()),this,SLOT(showHelp()));
    connect(about_action,SIGNAL(triggered()),this,SLOT(showAbout()));
    connect(settings_action,SIGNAL(triggered()),this,SLOT(showSettings()));

    connect(next_action,SIGNAL(triggered()),this,SLOT(nextImage()));
    connect(previous_action,SIGNAL(triggered()),this,SLOT(previousImage()));
    connect(save_action,SIGNAL(triggered()),this,SLOT(saveData()));
    connect(save_as_action,SIGNAL(triggered()),this,SLOT(saveDataAs()));
    connect(layoutmode_action,SIGNAL(triggered()),this,SLOT(modeLayout()));
    connect(linemode_action,SIGNAL(triggered()),this,SLOT(modeLines()));

    connect(viewport,SIGNAL(updateLabelCombobox(QString)),this,SLOT(updateLabelCombobox(QString)));
    connect(viewport,SIGNAL(updatePosition(int,int)),this,SLOT(updatePosition(int,int)));
    connect(label_combobox,SIGNAL(currentIndexChanged(QString)),viewport,SLOT(changeLabel(QString)));
    connect(viewport,SIGNAL(updateID(QString)),this,SLOT(updateID(QString)));
}

/* Este metode cal rescriurel en les labels noves */
QString MainWindow::nextLabel(QString currentLabel){
    int i = label_combobox->findText(currentLabel);
    i = (i+1)%label_combobox->count();
    return label_combobox->itemText(i);
}

void MainWindow::updateImgCount(){
    img_name->setText(data->get_image_Filename());
    file_number->setText(QString().setNum(current_file));
}

void MainWindow::closeEvent(QCloseEvent *event){
    if(!autosave && viewport->getmodified()){
        if(QMessageBox::Yes == QMessageBox::question(this,"Save changes?","Unsaved changes will be lost, do you want to save them?",
                                                     QMessageBox::Yes,QMessageBox::No))
            saveDocument();
    }else if(autosave)
        saveDocument();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    int key = event->key();
    switch(key){
        case Qt::Key_Left:
            previousImage();
            break;
        case Qt::Key_Right:
            nextImage();
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if(!filelist){
                QMessageBox::critical(this,"Error","No file list loaded");
                return;
            }
            bool ok;
            int prev_current_file;
            prev_current_file=current_file;
            current_file=file_number->text().toInt(&ok);
            if(!ok || current_file>=filelist->size() || current_file<0){
                QMessageBox::critical(this,"Error","Invalid image index");
                current_file=prev_current_file;
                return;
            }
            updateImgCount();
            loadDocument();
            viewport->loadImage(data->get_image_Filename());
            viewport->setRegions(data->get_regions());
            viewport->updateGL();
            break;
        case Qt::Key_Escape:
            viewport->makeCurrent();
            viewport->setFocus();
            break;
        case Qt::Key_F1:
            modeLayout();
            break;
        case Qt::Key_F2:
            modeLines();
            break;
        case Qt::Key_S: // Save
            if(event->modifiers() & Qt::ControlModifier){ // Save
              if(event->modifiers() & Qt::ShiftModifier) // Save as
                  saveDataAs();
              else // Regular save, overwrites
                  saveData();
            }
            break;
        default:
            QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::loadDocument(){
    if(data)
        delete data;
    data = new Document((*filelist)[current_file],this);
}

QString MainWindow::getSelectedLabel(){
    return label_combobox->currentText();
}

/*****************************************************/
/* Slots */
/*****************************************************/

void MainWindow::modeLayout(){
    mode = LAYOUT_MODE;
    viewport->changeMode(LAYOUT_MODE);
    viewport->updateGL();
}

void MainWindow::modeLines(){
    mode = LINE_MODE;
    viewport->changeMode(LINE_MODE);
    viewport->updateGL();
}

void MainWindow::showHelp(){

    help_window = new HelpWindow(this);

}

void MainWindow::showAbout(){

    about_window = new AboutWindow(this);

}

void MainWindow::showSettings(){
    QMap<QString, QVariant> prevset;
    prevset["autosave"] = autosave;
    prevset["overwrite"] = overwrite;
    settings_window = new SettingsWindow(this, &prevset);
    connect(settings_window,SIGNAL(updateSettings(QMap<QString,QVariant>&)),this,SLOT(updateSettings(QMap<QString,QVariant>&)));
}

void MainWindow::updateLabelCombobox(QString label){
   label_combobox->setCurrentIndex(label_combobox->findText(label));
}

void MainWindow::updatePosition(int x, int y){
    stringstream newpos;
    newpos << " x: " << x << " , y: " << y;
    pos_display->setText(QString::fromStdString(newpos.str()));
}

void MainWindow::loadFile(char* filename, bool single){
    QString fileName;
    if(filename==0)
        fileName=QFileDialog::getOpenFileName(this,"Open list file","","All files(*);;Text files (*.txt);;List files (*.ls *.list)");
    else
        fileName=QString::fromStdString(filename);

    if(single){
        if(filelist){
            if(!autosave && viewport->getmodified()){
                if(QMessageBox::Yes == QMessageBox::question(this,"Save changes?","Unsaved changes will be lost, do you want to save them?",
                                                             QMessageBox::Yes,QMessageBox::No))
                    saveDocument();
            }else if(autosave)
                saveDocument();
            delete filelist;
        }
        filelist= new QVector<QString>;
        filelist->append(fileName);
    }else{
        QFile *inputFile;
        QTextStream *inputStream;
        if(fileName!=""){
            inputFile = new QFile(fileName);
            if(!inputFile->open(QIODevice::ReadOnly)){
                QMessageBox::critical(this,"Error","Could not open the list file");
                delete inputFile;
                return;
            }
            inputStream = new QTextStream(inputFile);
            if(filelist){
                if(!autosave && viewport->getmodified()){
                    if(QMessageBox::Yes == QMessageBox::question(this,"Save changes?","Unsaved changes will be lost, do you want to save them?",
                                                                 QMessageBox::Yes,QMessageBox::No))
                        saveDocument();
                }else if(autosave)
                    saveDocument();
                delete filelist;
            }
            filelist= new QVector<QString>;
            while(!inputStream->atEnd()){
                QString newFileName;
                *inputStream>>newFileName;
		
		if (!inputStream->atEnd()){
		  filelist->push_back(newFileName);
		}
            }
            delete inputFile;
            delete inputStream;
        }
    }
    current_file=0;
    QString filetotal="/ ";
    filetotal.append(QString().setNum(filelist->size()-1));
    file_total->setText(filetotal);
    if(viewport){
        loadDocument();
        viewport->loadImage(data->get_image_Filename());
        viewport->setRegions(data->get_regions());
        viewport->updateGL();
    }
    updateImgCount();
}

void MainWindow::nextImage(){
    if(!filelist){
        QMessageBox::critical(this,"Error","No file list loaded");
        return;
    }
    if(!autosave && viewport->getmodified()){
        if(QMessageBox::Yes == QMessageBox::question(this,"Save changes?","Unsaved changes will be lost, do you want to save them?",
                                                     QMessageBox::Yes,QMessageBox::No))
            saveDocument();
    }else if(autosave)
        saveDocument();
    current_file++;

    if(current_file >= filelist->size()){
        QMessageBox::critical(this,"Error","Last image opened, there's no next image");
        current_file--;
        return;
    }
    loadDocument();
    viewport->loadImage(data->get_image_Filename());
    viewport->setRegions(data->get_regions());
    viewport->updateGL();
    updateImgCount();
}

void MainWindow::previousImage(){
    if(!filelist){
        QMessageBox::critical(this,"Error","No file list loaded");
        return;
    }
    if(!autosave && viewport->getmodified()){
        if(QMessageBox::Yes == QMessageBox::question(this,"Save changes?","Unsaved changes will be lost, do you want to save them?",
                                                     QMessageBox::Yes,QMessageBox::No))
            saveDocument();
    }else if(autosave)
        saveDocument();
    current_file--;
    if(current_file<0){
        QMessageBox::critical(this,"Error","First image opened, there's no previous image");
        current_file++;
        return;
    }
    loadDocument();
    viewport->loadImage(data->get_image_Filename());
    viewport->setRegions(data->get_regions());
    viewport->updateGL();
    updateImgCount();
}

void MainWindow::saveData(){
    bool ow;
    ow = overwrite;
    overwrite = true; // overwrite -> save
    if(data){
        saveDocument();
        viewport->setmodified(); // We've saved so we set to false the modification detection
    }
    overwrite = ow; // Restore value
}

void MainWindow::saveDataAs(){
    bool ow;
    ow = overwrite;
    overwrite = false; // Don't overwrite -> save as
    if(data){
        saveDocument();
        viewport->setmodified(); // We've saved so we set to false the modification detection
    }
    overwrite = ow; // Restore value
}

void MainWindow::saveDocument(){
    if(!data){
        QMessageBox::critical(this,"Error","No data to save");
        return;
    }
    QString xmlFileName = (*filelist)[current_file];
    if(!overwrite) // Ask for a new file name
        xmlFileName = QFileDialog::getSaveFileName(this,"Save xml file","","Text Files (*.txt);;All files(*)");

    data->saveFile(xmlFileName);
}

// Although all the settings are "bool" atributes, we use a QVariant to have a flexible interface
void MainWindow::updateSettings(QMap<QString, QVariant> &settings){
    autosave = settings["autosave"].toBool();
    overwrite = settings["overwrite"].toBool();
}

void MainWindow::updateID(QString id){
    id_selected_item->setText(id);
}

void MainWindow::loadSingleFile(){
    loadFile(0,true);
}
