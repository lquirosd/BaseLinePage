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

#ifndef GLVIEWPORT_H
#define GLVIEWPORT_H

#include <QtOpenGL/QGLWidget>
#include <QtGui/QWidget>
#include <QtGui/QWheelEvent>
#include <QtCore/QPoint>

#include "document.h"

class GLViewport : public QGLWidget
{
    Q_OBJECT

protected:

    GLuint tex_id;
    int img_width;
    int img_height;

    int selection_window;

    int win_width;
    int win_height;
    float zoom;
    float cam_x;
    float cam_y;

    bool unfinished_figure;
    int next_point;
    TextLine* selected_line;
    int selected_line_index;
    TextRegion* selected_region;
    int selected_region_index;
    Point* selected_point;
    int selected_point_index;
    int selected_point_type;

    int last_line_ID_index;
    int last_region_ID_index;

    bool modified;

    QPoint last_pos;

    QWidget *main_window;

    QVector<TextRegion> *regions;
    QVector<TextLine> *lines;
    QVector<QString> *used_line_IDs;
    QVector<QString> *used_region_IDs;

    QVector<Handle> *region_handles;
  //  QVector<Handle> *line_handles;

    Handle *closest_handle;
    Handle *selected_handle;

    int mode;

    QString generateNewID(QString type); /* v.g. "r" for region or "l" for line */
    void updateSelectionWindow(){ selection_window=zoom*5;
                                  if(selection_window<5)
                                    selection_window=5; }

    void calculateRegionHandles();
    void recalculateRegionHandles();

public:

    explicit GLViewport(QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    bool getunfinished_figure(){return unfinished_figure;}
    bool getmodified(){return modified;}
    void setmodified(){modified = false;}
    int getimg_width(){return img_width;}
    int getimg_height(){return img_height;}

    void loadImage(QString imageFileName);

    void setRegions(QVector<TextRegion> *nregions);

    void keyPressEvent(QKeyEvent *event);

signals:

    void updatePosition(int x, int y);
    void updateLabelCombobox(QString label);
    void updateID(QString id);
    
public slots:
    void changeMode(int m);
    void changeLabel(QString text);
};

#endif // GLVIEWPORT_H
