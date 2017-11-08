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

#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QtGui/QDialog>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#define ABOUT_L1     "Layout and line supervisor for correcting automatic classification.  \n"
#define ABOUT_L2     "  Created on: 2013 \n"
#define ABOUT_L3     "  Author: Jorge Martinez Vargas \n"
#define ABOUT_L4     "      jormarv5@fiv.upv.es \n"



class AboutWindow : public QDialog
{
    Q_OBJECT
    /* Atributes */
    QTextEdit *about_text;
    QVBoxLayout *about_layout;
public:
    explicit AboutWindow(QWidget *parent = 0, QVector<QString> *text = 0);
    
signals:
    
public slots:
    
};

#endif // ABOUTWINDOW_H
