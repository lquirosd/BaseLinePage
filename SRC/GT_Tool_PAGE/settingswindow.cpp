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

#include <QMap>
#include <QVariant>

#include <iostream>

#include "settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent, QMap<QString, QVariant> *prevset) :
    QDialog(parent)
{
    main_window = parent;
    if(prevset==0){ // No previous settings, default ones loaded (this shouldn't happen anyways)
        autosave = false;
        overwrite = true;
    }else{
        autosave = (*prevset)["autosave"].toBool();
        overwrite = (*prevset)["overwrite"].toBool();
    }

    /* GUI creation */
    setAttribute(Qt::WA_DeleteOnClose);

    settings_layout = new QVBoxLayout;
    overwrite_checkbox = new QCheckBox("Overwrite xml file when saving");
    autosave_checkbox = new QCheckBox("Autosave changes on image change/exit");
    settings_layout->addWidget(autosave_checkbox);
    settings_layout->addWidget(overwrite_checkbox);
    commands_layout = new QHBoxLayout;
    apply_button = new QPushButton("Apply");
    accept_button = new QPushButton("Accept");
    cancel_button = new QPushButton("Cancel");
    commands_widget = new QWidget();
    commands_layout->addWidget(apply_button);
    commands_layout->addWidget(accept_button);
    commands_layout->addWidget(cancel_button);
    commands_widget->setLayout(commands_layout);
    settings_layout->addWidget(commands_widget);
    this->setLayout(settings_layout);

    if(autosave)
        autosave_checkbox->setChecked(true);
    else
        autosave_checkbox->setChecked(false);
    if(overwrite)
        overwrite_checkbox->setChecked(true);
    else
        overwrite_checkbox->setChecked(false);

    connect(autosave_checkbox,SIGNAL(toggled(bool)),this,SLOT(updateAutosave(bool)));
    connect(overwrite_checkbox,SIGNAL(toggled(bool)),this,SLOT(updateOverwrite(bool)));
    connect(apply_button,SIGNAL(clicked()),this,SLOT(applySlot()));
    connect(accept_button,SIGNAL(clicked()),this,SLOT(acceptSlot()));
    connect(cancel_button,SIGNAL(clicked()),this,SLOT(close()));
    show();
}

/**********************************************************************************
                                  SLOTS
***********************************************************************************/

void SettingsWindow::applySlot(){
    QMap<QString, QVariant> settings;
    settings["autosave"] = autosave;
    settings["overwrite"] = overwrite;
    updateSettings(settings);
}

void SettingsWindow::acceptSlot(){
    QMap<QString, QVariant> settings;
    settings["autosave"] = autosave;
    settings["overwrite"] = overwrite;
    updateSettings(settings);
    close();
}

void SettingsWindow::updateAutosave(bool s){
    autosave = s;
}

void SettingsWindow::updateOverwrite(bool s){
    overwrite = s;
}

void SettingsWindow::closeEvent(QCloseEvent *event){
    disconnect(this,SIGNAL(updateSettings(QMap<QString,QVariant>&)),main_window,SLOT(updateSettings(QMap<QString,QVariant>&)));
}

