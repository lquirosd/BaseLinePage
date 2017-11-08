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

#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QtGui/QDialog>
#include <QtGui/QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QWidget>

class SettingsWindow : public QDialog
{
    Q_OBJECT

    /* Atributes */
    QWidget *main_window;
    bool autosave;
    bool overwrite;

    /* GUI atributes */
    QVBoxLayout *settings_layout;
    QCheckBox *overwrite_checkbox;
    QCheckBox *autosave_checkbox;
    QHBoxLayout *commands_layout;
    QWidget *commands_widget;
    QPushButton *apply_button;
    QPushButton *accept_button;
    QPushButton *cancel_button;

    void closeEvent(QCloseEvent *event);

public:
    explicit SettingsWindow(QWidget *parent = 0, QMap<QString,QVariant> *prevset = 0);

signals:
    void updateSettings(QMap<QString, QVariant> &settings);

public slots:
    void applySlot();
    void acceptSlot();
    void updateAutosave(bool s);
    void updateOverwrite(bool s);

};

#endif // SETTINGSWINDOW_H
