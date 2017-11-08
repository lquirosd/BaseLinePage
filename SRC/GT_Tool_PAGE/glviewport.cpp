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
#include <QMessageBox>
#include <QTextStream>
#include <QDataStream>
#include <QFile>
#include <QImage>

#include <fstream>
#include <string>
#include <sstream>

#include <iostream>

#include "mainwindow.h"
#include "glviewport.h"


using namespace std;

GLViewport::GLViewport(QWidget *parent) :
    QGLWidget(parent)
{
    main_window=parent;
    //mode = LAYOUT_MODE;
    mode = LINE_MODE;
    setMouseTracking(true); // Mouse tracking for handle detection
    zoom=1;
    cam_x = cam_y = 0;
    win_width=1500;
    win_height=1200;
    unfinished_figure = false;
    next_point=0;
    selected_line=0;
    selected_line_index=-1;
    selected_region=0;
    selected_region_index=-1;
    selected_point=0;
    selected_point_index=0;
    selected_point_type=-1;
    regions=0;
    lines=0;
    last_line_ID_index=0;
    last_region_ID_index=0;
    used_line_IDs=0;
    used_region_IDs=0;
    region_handles =0;
 //   line_handles = 0;
    closest_handle = 0;
    selected_handle = 0;
    modified=false;
    setFixedSize(500,200);
    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
}

void GLViewport::initializeGL(){
  //  Set up the rendering context
    glClearColor(1.0,1.0,1.0,0.0);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GLViewport::resizeGL(int w, int h){
    // Set up viewport, projection, etc
    glViewport(0, 0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    win_width=w;
    win_height=h;
    glOrtho(-(win_width/2.0)*zoom,(win_width/2.0)*zoom,-(win_height/2.0)*zoom,(win_height/2.0)*zoom,-10,10);
    glMatrixMode(GL_MODELVIEW);
}

void GLViewport::paintGL(){
    // Draw the scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(-cam_x,-cam_y,0);
    // Draw original image as a texture
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D,tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glPushMatrix();
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-img_width/2, -img_height/2, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (img_width/2,-img_height/2 , 0.0);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (img_width/2, img_height/2, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-img_width/2, img_height/2, 0.0);

    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    if(regions){
      if(mode==LAYOUT_MODE){// || mode==LINE_MODE){
            // Draw regions
            glLineWidth(1);
            int num_regs;
            if(unfinished_figure)
                num_regs=regions->size()-1;
            else
                num_regs=regions->size();
	    
            for(int i=0;i<num_regs;i++){
                glPushMatrix();
                glColor3f(1,0,1);
                glBegin(GL_LINE_LOOP);
                for(int j=0;j<(*regions)[i].coords.size();j++){
                    int posx, posy;
                    posx = (*regions)[i].coords[j].x - img_width/2;
                    posy = img_height/2 - (*regions)[i].coords[j].y;
                    glVertex3f(posx, posy, 1);
                }
                glEnd();
                glPopMatrix();
            }
	    
            // Draw points
           
            for(int i=0;i<num_regs;i++){
	       glPushMatrix();
	       glPointSize(6);
	       glBegin(GL_POINTS);
                for(int j=0;j<(*regions)[i].coords.size();j++){
                    int posx,posy;
                    posx = (*regions)[i].coords[j].x - img_width/2;
                    posy = img_height/2 - (*regions)[i].coords[j].y; // (h - y) - h/2
                    glColor3f(0,0,1);
                    glVertex3f(posx,posy,1);
                }
		 glEnd();
		 glPopMatrix();
            }
	    
            // for(int i=0;i<next_point;i++){
            //     int posx,posy;
            //     posx = (*regions)[num_regs].coords[i].x - img_width/2;
            //     posy = img_height/2 - (*regions)[num_regs].coords[i].y; // (h - y) - h/2
            //     glColor3f(0,0,1);
            //     glVertex3f(posx,posy,1);
            // }
            // glEnd();
            //glPopMatrix();
	    
            // Draw selected region
            if(selected_region){	      
                glLineWidth(4);
                glPushMatrix();
                glColor3f(0.5,0,0.5);
                glBegin(GL_LINE_LOOP);
                for(int i=0;i<selected_region->coords.size();i++){
                    int posx, posy;
                    posx = selected_region->coords[i].x - img_width/2;
                    posy = img_height/2 -  selected_region->coords[i].y;
                    glVertex3f(posx, posy, 1);
                }
                glEnd();
                glPopMatrix();
            }
	    
            // Draw selected point
            glLineWidth(1);
            glPushMatrix();
            glColor3f(1,0,0);
            glBegin(GL_LINE_LOOP);
            if(selected_point){
                int posx = selected_point->x - img_width/2;
                int posy = img_height/2 - selected_point->y; // (h - y) - h/2
                glVertex3f(posx-selection_window,posy-selection_window,1.2);
                glVertex3f(posx+selection_window,posy-selection_window,1.2);
                glVertex3f(posx+selection_window,posy+selection_window,1.2);
                glVertex3f(posx-selection_window,posy+selection_window,1.2);
            }
            glEnd();
            glPopMatrix();
	    
            // Draw handle
            glPointSize(6);
            glPushMatrix();
            glColor3f(0,1,0.25);
            glBegin(GL_POINTS);
            if(closest_handle){
                int posx = closest_handle->x - img_width/2;
                int posy = img_height/2 - closest_handle->y; // (h - y) - h/2
                glVertex3f(posx,posy,1.2);
            }
            glEnd();
            glPopMatrix();
        }
        if(mode==LINE_MODE){
            // Draw lines (textlines & baselines)
            if(lines){
	      glLineWidth(1);
	      for (int reg = 0; reg < regions->size(); reg++){
		lines=&(*regions)[reg].lines;
		for(int i=0;i<(*lines).size();i++){
		  // Baselines
		  glPushMatrix(); 
		  glColor3f(0,0,1);
		  glBegin(GL_LINE_STRIP);
		  for(int j=0;j<(*lines)[i].baseline.size();j++){
		    int posx, posy;
		    posx = (*lines)[i].baseline[j].x - img_width/2;
		    posy = img_height/2 - (*lines)[i].baseline[j].y;
		    glVertex3f(posx,posy,1);
		  }
		  glEnd();
		  glPopMatrix();
		  
		  // Draw line poligons
		  //TextLines
		  // glPushMatrix();
		  // glColor3f(0,0.5,1);
		  // glBegin(GL_LINE_LOOP);
		  // for(int j=0;j<(*lines)[i].coords.size();j++){
		  //     int posx, posy;
		  //     posx = (*lines)[i].coords[j].x - img_width/2;
		  //     posy = img_height/2 - (*lines)[i].coords[j].y;
		  //     glVertex3f(posx,posy,1);
		  // }
		  // glEnd();
		  // glPopMatrix();
		  
		}
		
	      }
	      
	      // Draw points	                      
	      for (int reg = 0; reg < regions->size(); reg++){
		glPushMatrix();
		glPointSize(5);
		glColor3f(0.5,0,1);
		glBegin(GL_POINTS);
		lines=&(*regions)[reg].lines;
		for(int i=0;i<lines->size();i++){
                    // Baselines
                    for(int j=0;j<(*lines)[i].baseline.size();j++){
                        int posx, posy;
                        posx = (*lines)[i].baseline[j].x - img_width/2;
                        posy = img_height/2 - (*lines)[i].baseline[j].y;
                        glVertex3f(posx,posy,1);
                    }
                    // // TextLines poligon
                    // for(int j=0;j<(*lines)[i].coords.size();j++){
                    //     int posx, posy;
                    //     posx = (*lines)[i].coords[j].x - img_width/2;
                    //     posy = img_height/2 - (*lines)[i].coords[j].y;
                    //     glVertex3f(posx,posy,1);
                    // }
		  }
		
		  glEnd();
		  glPopMatrix();
		}
            }
            // Draw selected line
            if(selected_line){
                glLineWidth(2);
                // Baseline
                glPushMatrix();
                glColor3f(1,0,0);
                glBegin(GL_LINE_STRIP);
                for(int j=0;j<selected_line->baseline.size();j++){
                    int posx, posy;
                    posx = selected_line->baseline[j].x - img_width/2;
                    posy = img_height/2 - selected_line->baseline[j].y;
                    glVertex3f(posx,posy,1);
                }
                glEnd();
                glPopMatrix();
		// Draw line poligon
                // TextLine
                // glPushMatrix();
                // glColor3f(0.25,0.5,1);
                // glBegin(GL_LINE_LOOP);
                // for(int j=0;j<selected_line->coords.size();j++){
                //     int posx, posy;
                //     posx = selected_line->coords[j].x - img_width/2;
                //     posy = img_height/2 - selected_line->coords[j].y;
                //     glVertex3f(posx,posy,1);
                // }
                // glEnd();
                // glPopMatrix();
            }
        }
    }

    glEnd();
    glPopMatrix();
    glFlush();
}

void GLViewport::calculateRegionHandles(){
    if(region_handles)
        delete region_handles;
    region_handles = new QVector<Handle>;
    for(int i=0;i<regions->size();i++){
        Coords * c = &((*regions)[i].coords);
        for(int j=1;j<c->size();j++){
            region_handles->append(Handle(&((*c)[j-1]),&((*c)[j])));
        }
        if(c->size()>1)
            region_handles->append(Handle(&((*c)[c->size()-1]),&((*c)[0])));
    }
}

void GLViewport::recalculateRegionHandles(){
    if(region_handles){
        for(int i=0;i<region_handles->size();i++){
            (*region_handles)[i].recalculate();
        }
    }
}

void GLViewport::setRegions(QVector<TextRegion> *nregions){
    regions = nregions;
    next_point = 0;
    unfinished_figure = false;
    if(used_line_IDs)
        delete used_line_IDs;
    if(used_region_IDs)
        delete used_region_IDs;
    used_line_IDs = new QVector<QString>;
    used_region_IDs = new QVector<QString>;
    last_line_ID_index=0;
    last_region_ID_index=regions->size();
    for(int i=0;i<regions->size();i++){
        used_region_IDs->append((*regions)[i].id);
        for(int j=0;j<(*regions)[i].lines.size();j++){
            last_line_ID_index++;
            used_line_IDs->append((*regions)[i].lines[j].id);
        }
    }

    if(mode == LINE_MODE){
        MainWindow *mw = (MainWindow*) main_window;
        selected_region = &(*regions)[0];
        selected_region_index = 0;
        mw->updateID(selected_region->id);
        lines = &selected_region->lines;
        selected_line=0;
        selected_line_index=-1;
    }
    calculateRegionHandles();
}

void GLViewport::loadImage(QString imageFileName){
    QImage *image = new QImage();
    if(!image->load(imageFileName)){
        QMessageBox::critical(this,"Error","Could not open image file");
        return;
    }

    QImage GL_formated_image;

    img_width=image->width();
    img_height=image->height();

    int tex_width=2*img_width;
    int tex_height=2*img_height;
    int w_ok, h_ok;
    do{
        tex_width=tex_width/2;
        tex_height=tex_height/2;
        glTexImage2D(GL_PROXY_TEXTURE_2D,0,GL_RGBA,tex_width,tex_height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w_ok);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h_ok);
    }while(w_ok==0 || h_ok == 0);

    GL_formated_image = QGLWidget::convertToGLFormat(image->scaled(tex_width,tex_height));

    glDeleteTextures(1,&tex_id); // Delete the previous texture
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&tex_id); // Obtain a free texture name
    glBindTexture(GL_TEXTURE_2D,tex_id);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA,
                  GL_formated_image.width(), GL_formated_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                  GL_formated_image.bits());
    glDisable(GL_TEXTURE_2D);

    float w = float(img_width)/win_width;
    float h = float(img_height)/win_height;

    if(w>h)
        zoom=w;
    else
        zoom=h;
    cam_x=cam_y=0;

    updateSelectionWindow();
    modified = false;
    selected_region = 0;
    selected_line = 0;
    selected_point = 0;
    selected_region_index = selected_line_index = -1;
    if(used_line_IDs)
        delete used_line_IDs;
    if(used_region_IDs)
        delete used_region_IDs;
    used_line_IDs = 0;
    used_region_IDs = 0;
    last_line_ID_index = 0;
    last_region_ID_index = 0;
    closest_handle = 0;
    selected_handle = 0;
    resizeGL(win_width,win_height);

    delete image;
}

void GLViewport::wheelEvent(QWheelEvent *event){
    if(regions){
        if(event->orientation() == Qt::Vertical){
           float delta = zoom/(event->delta());
           zoom -= delta*10;
           updateSelectionWindow();
           glMatrixMode(GL_PROJECTION);
           glLoadIdentity();
           glOrtho(-(win_width/2.0)*zoom,(win_width/2.0)*zoom,-(win_height/2.0)*zoom,(win_height/2.0)*zoom,-10,10);
           updateGL();
       }
    }
}

void GLViewport::mousePressEvent(QMouseEvent *event){
    setFocus();
    last_pos = event->pos();
    MainWindow *mw;
    mw = (MainWindow*) main_window;
    if(regions){
        if(event->button() ==  Qt::MidButton){ // Camera reset
            float w = float(img_width)/win_width;
            float h = float(img_height)/win_height;
            if(event->modifiers() & Qt::ControlModifier){
                if(w>h)
                    zoom=w;
                else
                    zoom=h;
            }else{
                zoom=w;
            }
            updateSelectionWindow();
            cam_x=cam_y=0;
            resizeGL(win_width,win_height);
            updateGL();
        }
        else if(event->button() == Qt::LeftButton){
            int pos_x,pos_y;
            pos_x= zoom*(event->x() - win_width/2) + cam_x;
            pos_y= zoom*(win_height/2 - event->y()) + cam_y;
            if(mode == LAYOUT_MODE){
                if(event->modifiers() & Qt::ControlModifier){
                    selected_region = 0;
                    selected_region_index = -1;
                    selected_point = 0;
                    selected_handle = 0;
                    if(unfinished_figure){ // Add point to unfinished layout
                        (*regions)[regions->size()-1].coords[next_point].x = pos_x + img_width/2;
                        (*regions)[regions->size()-1].coords[next_point].y = img_height/2 - pos_y;
                        next_point = (next_point+1)%NUM_VERT;
                        if(next_point==0){
                            unfinished_figure = false;
                            calculateRegionHandles();
                        }

                    }else{ // Start new layout
                        MainWindow *mw = (MainWindow*)main_window;
                        TextRegion newregion;
                        newregion.type = mw->getSelectedLabel();
                        newregion.id = generateNewID("r");
                        newregion.coords.resize(4);
                        newregion.coords[0].x = pos_x + img_width/2;
                        newregion.coords[0].y = img_height/2 - pos_y;
                        unfinished_figure = true;
                        next_point = 1;
                        regions->push_back(newregion);
                    }
                    modified = true;
                }else if(event->modifiers() & Qt::ShiftModifier){ // Label change
                    if(selected_region){
                        selected_point = 0;
                        selected_region->type = mw->nextLabel(selected_region->type);
                        modified = true;
                        updateLabelCombobox(selected_region->type);
                    }
                }else{ // Region/point/handle selection
                    selected_region=0;
                    selected_region_index = -1;
                    selected_point=0;
                    selected_point_index=0;
                    selected_point_type=-1;
                    selected_handle = 0;
                    for(int reg=0; reg<regions->size(); reg++){
                        for(int j=0;j<(*regions)[reg].coords.size();j++){
                            int data_x = (*regions)[reg].coords[j].x - img_width/2;
                            int data_y = img_height/2 - (*regions)[reg].coords[j].y;
                            if(data_x-selection_window<pos_x && pos_x<data_x+selection_window
                               && data_y-selection_window<pos_y && pos_y<data_y+selection_window){
                                selected_point = &(*regions)[reg].coords[j];
                                selected_region = &(*regions)[reg];
                                mw->updateID(selected_region->id);
                                updateLabelCombobox(selected_region->type);
                                selected_region_index = reg;
                                updateLabelCombobox(selected_region->type);
                                if(unfinished_figure && reg==regions->size()-1){
                                    selected_region = 0;
                                    selected_region_index = -1;
                                }
                                updatePosition(selected_point->x, selected_point->y);
                                break;
                            }
                        }
                    }
                    if(closest_handle){
                        int data_x = closest_handle->x - img_width/2;
                        int data_y = img_height/2 - closest_handle->y;
                        if(data_x-selection_window<pos_x && pos_x<data_x+selection_window
                           && data_y-selection_window<pos_y && pos_y<data_y+selection_window){
                            selected_handle = closest_handle;
                            updatePosition(selected_handle->x, selected_handle->y);
                        }
                    }
                }
            }else if(mode == LINE_MODE){
                if(event->modifiers() & Qt::ControlModifier){
                    selected_point = 0;
                    selected_point_index = -1;
                    selected_point_type = -1;
                    selected_line = 0;
                    selected_line_index = -1;
                    int nx = pos_x + img_width/2;
                    int ny = img_height/2 - pos_y;
                    if(event->modifiers() & Qt::ShiftModifier){ // New line
                        TextLine nline;
                        nline.id = generateNewID("l");
                        nline.baseline.append(Point(nx, ny));
                        selected_region->lines.append(nline);
                        modified = true;
                    }else{ // Add point to existing lines
                        bool exist_lines = false;
                        int closest_line=0;
			int closest_region=0;
                        int index=0;
                        int dist=99999999;
			for (int reg = 0; reg < regions->size(); reg++) {			  

			  selected_region = &(*regions)[reg];
			  for(int i=0; i<selected_region->lines.size(); i++){
                            exist_lines = true;
                            int j=0;
                            while(j < selected_region->lines[i].baseline.size() && nx > selected_region->lines[i].baseline[j].x)
			      j++;
                            int indtemp = j;
                            if(j==selected_region->lines[i].baseline.size() || (j > 0 &&
                               selected_region->lines[i].baseline[j].x - nx > nx - selected_region->lines[i].baseline[j-1].x))
			      j--;
                            int ndist = ny - selected_region->lines[i].baseline[j].y;
			    
                            if(ndist < 0)
                                ndist = ndist*(-1);
                            if(ndist <= dist){
			      dist = ndist;
			      closest_line = i;
			      closest_region=reg;
			      index = indtemp;
                            }
			  }
			}
                        if(exist_lines){
			  selected_region = &(*regions)[closest_region];
			  selected_region->lines[closest_line].baseline.insert(index,Point(nx,ny));
			  modified = true;
                        }else{
                            QMessageBox::critical(this,"Error","No lines loaded. Can't add points");
                        }
                    }
                } else {
                 // Line selection
                    selected_point = 0;
                    selected_line = 0;
		    selected_region = 0;
		    TextRegion* region = 0;
                    selected_line_index = -1;
                    if(regions){
		      for (int reg = 0; reg < regions->size(); reg++) {			  

			region = &(*regions)[reg];
                        for(int i = 0; i < region->lines.size(); i++){
                        // Baseline
                            for(int j=0; j<region->lines[i].baseline.size();j++){
                                int data_x = region->lines[i].baseline[j].x - img_width/2;
                                int data_y = img_height/2 - region->lines[i].baseline[j].y;
                                if(data_x-selection_window<pos_x && pos_x<data_x+selection_window
                                   && data_y-selection_window<pos_y && pos_y<data_y+selection_window){
				  //selected_point = &((*lines)[i].baseline[j]);
				    selected_point = &(region->lines[i].baseline[j]);
                                    selected_point_index = j;
                                    selected_point_type = BASELINE_POINT;
                                    selected_line_index = i;
                                    //selected_line = &((*lines)[i]);
				    selected_line = & (region->lines[i]);
				    selected_region = region;
							  
                                    updatePosition(selected_point->x, selected_point->y);
                                    break;
                                }
                            }
			}
                        // TextLine
                            // for(int j=0;j<(*lines)[i].coords.size();j++){
                            //     int data_x = (*lines)[i].coords[j].x - img_width/2;
                            //     int data_y = img_height/2 - (*lines)[i].coords[j].y;
                            //     if(data_x-selection_window<pos_x && pos_x<data_x+selection_window
                            //        && data_y-selection_window<pos_y && pos_y<data_y+selection_window){
                            //         selected_point = &((*lines)[i].coords[j]);
                            //         selected_point_index = j;
                            //         selected_point_type = TEXTLINE_POINT;
                            //         selected_line_index = i;
                            //         selected_line = &((*lines)[i]);
                            //         updatePosition(selected_point->x, selected_point->y);
                            //         break;
                            //     }
                            // }
                        }
                        if(selected_line)
                            mw->updateID(selected_line->id);
                    }
                }
            }
        }
        updateGL();
    }
}

void GLViewport::mouseMoveEvent(QMouseEvent *event){
    if(regions){
        int dx = event->x() - last_pos.x();
        int dy = event->y() - last_pos.y();
        int pos_x,pos_y;
        pos_x= zoom*(event->x() - win_width/2) + cam_x;
        pos_y= zoom*(win_height/2 - event->y()) + cam_y;
        if(mode == LAYOUT_MODE && region_handles){
            closest_handle=0;
            for(int i = 0;i<region_handles->size(); i++){
                int window = selection_window*3;
                int data_x = (*region_handles)[i].x - img_width/2;
                int data_y = img_height/2 - (*region_handles)[i].y;
                if(data_x-window<pos_x && pos_x<data_x+window
                   && data_y-window<pos_y && pos_y<data_y+window){
                    closest_handle = &((*region_handles)[i]);
                }	
            }
        }

 	
        float units_per_pixel = zoom;
        if(event->buttons() & Qt::RightButton){ // Camera movement
            cam_x-=dx*units_per_pixel;
            cam_y+=dy*units_per_pixel;
        } else if(event->buttons() & Qt::LeftButton){ 
	  if(mode == LAYOUT_MODE) {
	    
	    if(selected_point){
	      selected_point->x = pos_x + img_width/2;
	      selected_point->y = img_height/2 - pos_y;
	      updatePosition(selected_point->x, selected_point->y);
	      modified = true;
	    }
	    if(selected_handle){
	      selected_handle->move(pos_x+ img_width/2, img_height/2 - pos_y);
	      updatePosition(selected_handle->x, selected_handle->y);
	      modified = true;
	    }
	    recalculateRegionHandles();
	    
	  } else if(mode == LINE_MODE){
	    if(selected_point){
	      
	      if(event->modifiers() & Qt::ShiftModifier){
		int dx = pos_x + img_width/2 - selected_point->x;
		int dy = img_height/2 - pos_y - selected_point->y;
		for(int i=0;i<selected_line->baseline.size();i++){
		  selected_line->baseline[i].x += dx;
		  selected_line->baseline[i].y += dy;
		}
		for(int i=0;i<selected_line->coords.size();i++){
		  selected_line->coords[i].x += dx;
		  selected_line->coords[i].y += dy;
		}
	      } else{
		selected_point->x = pos_x + img_width/2;
		selected_point->y = img_height/2 - pos_y;
		if(selected_point_type == BASELINE_POINT){
		  if(selected_point_index > 0 &&
		     selected_point->x <= selected_line->baseline[selected_point_index-1].x){
		    selected_point->x = selected_line->baseline[selected_point_index-1].x +1;
		  }else if(selected_point_index < selected_line->baseline.size()-1 &&
			   selected_point->x >= selected_line->baseline[selected_point_index+1].x){
		    selected_point->x = selected_line->baseline[selected_point_index+1].x -1;
		  }
		}
	      }
	      
	      updatePosition(selected_point->x, selected_point->y);
	      modified = true;
	    } 
	  }
	}
        
        last_pos = event->pos();
        updateGL();
    }
}

void GLViewport::keyPressEvent(QKeyEvent *event){
    int key = event->key();
   
    switch(key){
    case Qt::Key_Space:
        if(selected_line_index>=0){
  
	  selected_line_index = (selected_line_index+1) % lines->size();
	  //selected_line = &((*lines)[selected_line_index]);
	  selected_line = & (selected_region->lines[selected_line_index]);
	    
        }
        break;
    case Qt::Key_Up:
        if(selected_line){
            for(int i=0;i<selected_line->baseline.size();i++){
                selected_line->baseline[i].y = selected_line->baseline[i].y - zoom;
            }
            updatePosition(selected_point->x, selected_point->y);
            modified = true;
        }
        break;
    case Qt::Key_Down:
        if(selected_line){
            for(int i=0;i<selected_line->baseline.size();i++){
                selected_line->baseline[i].y = selected_line->baseline[i].y + zoom;
            }
            updatePosition(selected_point->x, selected_point->y);
            modified = true;
        }
        break;
    case Qt::Key_Delete:
       if(mode == LAYOUT_MODE){
            if(selected_region){
                regions->remove(selected_region_index);
                selected_region=0;
                selected_region_index=-1;
                selected_point=0;
                selected_point_index=0;
                selected_point_type=-1;
                selected_handle = 0;
                closest_handle = 0;
                recalculateRegionHandles();
                modified = true;
            }
        }else if(mode == LINE_MODE){
            if(selected_line){
                if(event->modifiers() & Qt::ShiftModifier){ // Delete the whole line
		  (*selected_region).lines.remove(selected_line_index);
                    if((*selected_region).lines.size() > 0){
		      //selected_line_index = selected_line_index%lines->size();
		      selected_line_index = selected_line_index % (*selected_region).lines.size();
                        //selected_line = &((*lines)[selected_line_index]);
			selected_line = & (selected_region->lines[selected_line_index]);

                    }else{
                        selected_line_index = -1;
                        selected_line = 0;
                    }
                }else if(selected_point){ // Delete only the selected point
                    switch(selected_point_type){
                    case BASELINE_POINT:
                        selected_line->baseline.remove(selected_point_index);
                        break;
                    case TEXTLINE_POINT:
                        selected_line->coords.remove(selected_point_index);
                        break;
                    }
                    selected_point=0;
                    selected_point_index=-1;
                    selected_point_type=-1;
                    if(selected_line->baseline.size()==0 && selected_line->coords.size()==0){
                        selected_region->lines.remove(selected_line_index);
                        if(lines->size() > 0){
                            selected_line_index = selected_line_index%lines->size();
                            selected_line = &((*lines)[selected_line_index]);
                        }else{
                            selected_line_index = -1;
                            selected_line = 0;
                        }
                    }
                modified = true;
                }
            }
        }
    default:
        QGLWidget::keyPressEvent(event);
    }
    updateGL();
}

QString GLViewport::generateNewID(QString type){
    QString newID="";
    if(type == "r"){ // Region
        do{
            last_region_ID_index++;
            newID="r";
            newID.append(QString::number(last_region_ID_index));
        }while(used_region_IDs->contains(newID));
    }else if(type == "l"){ // Line
        do{
            last_line_ID_index++;
            newID="l";
            newID.append(QString::number(last_line_ID_index));
        }while(used_line_IDs->contains(newID));
    }else{ // Not one of the indicators we use, we don't know what the new ID is for
        int new_index;
        if(last_line_ID_index>last_region_ID_index)
            new_index = last_line_ID_index;
        else
            new_index = last_region_ID_index;
        do{
            new_index++;
            newID=type;
            newID.append(QString::number(new_index));
        }while(used_region_IDs->contains(newID) || used_line_IDs->contains(newID));
    }
    return newID;
}

/*****************************************************/
/* Slots */
/*****************************************************/

void GLViewport::changeMode(int m){
    MainWindow *mw;
    mw = (MainWindow*) main_window;
    closest_handle = 0;
    selected_handle = 0;
    if(regions){
        if(mode == LAYOUT_MODE && m == LINE_MODE){
            selected_point = 0;
            if(!selected_region){
                if(regions->size() > 0){
                    selected_region = &(*regions)[0];
                    selected_region_index = 0;
                    mw->updateID(selected_region->id);
                    updateLabelCombobox(selected_region->type);
                }else{
                    selected_region = 0;
                    selected_region_index = -1;
                }
            }
            if(selected_region)
                lines = &selected_region->lines;
            else
                lines = 0;
            selected_line=0;
            selected_line_index=-1;
        }else if(mode == LINE_MODE && m == LAYOUT_MODE){
            selected_point = 0;
            lines = 0;
            selected_line=0;
            selected_line_index=-1;
            if(selected_region){
                mw->updateID(selected_region->id);
                updateLabelCombobox(selected_region->type);
            }
        }
    }
    mode = m;
}

void GLViewport::changeLabel(QString text){
    if(selected_region){
        selected_region->type=text;
        modified=true;
    }
}
