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


/* Document class for parsing PAGE XML documents, modifying them and writing them back */
/* Disclaimer: This class gives limited support to the PAGE XML format, meaning it */
/* parses and writes a subset of the format available elements and attributes. */
/* Use it at your own risk. */
/* The code is segmented into methods as much as possible to make easy the ampliation */
/* of the supported features and/or the adaptation to changes in the PAGE XML format */

#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <iostream>

#include "document.h"

using namespace std;

// Handle
Handle::Handle(){x=y=0;p1=p2=0;}

Handle::Handle(Point *np1, Point *np2){
    p1 = np1;
    p2 = np2;
    x = p1->x + (p2->x - p1->x)/2;
    y = p1->y + (p2->y - p1->y)/2;
}

Handle::Handle(const Handle &other){
    x = other.x;
    y = other.y;
    p1 = other.p1;
    p2 = other.p2;
}

Handle& Handle::operator =(const Handle &rhs){
    x=rhs.x;
    y=rhs.y;
    p1=rhs.p1;
    p2=rhs.p2;
    return *this;
}

void Handle::move(int nx, int ny){
    int dx;
    int dy;
    dx = nx - x;
    dy = ny - y;
    p1->x = p1->x + dx;
    p1->y = p1->y + dy;
    p2->x = p2->x + dx;
    p2->y = p2->y + dy;
    x = nx;
    y = ny;
}

void Handle::recalculate(){
    x = p1->x + (p2->x - p1->x)/2;
    y = p1->y + (p2->y - p1->y)/2;
}

// Point
Point::Point(){x=y=0;}

Point::Point(const Point &other){
    x=other.x;
    y=other.y;
}

Point::Point(int nx, int ny){
    x = nx;
    y = ny;
}

Point& Point::operator =(const Point &rhs){
    x=rhs.x;
    y=rhs.y;
    return *this;
}

// TextEquiv
TextEquiv::TextEquiv(){
    plainText = "";
    unicode = "";
}

TextEquiv::TextEquiv(const TextEquiv &other){
    plainText = other.plainText;
    unicode = other.unicode;
}

TextEquiv& TextEquiv::operator =(const TextEquiv &rhs){
    plainText = rhs.plainText;
    unicode = rhs.unicode;
    return *this;
}

// TextLine
TextLine::TextLine(){
    id="";
}

TextLine::TextLine(const TextLine &other){
    id = other.id;
    coords = other.coords;
    baseline = other.baseline;
    textEquiv = other.textEquiv;
}

TextLine& TextLine::operator =(const TextLine &rhs){
    id = rhs.id;
    coords = rhs.coords;
    baseline = rhs.baseline;
    textEquiv = rhs.textEquiv;
    return *this;
}

// TextRegion
TextRegion::TextRegion(){
    id="";
    type="";
}

TextRegion::TextRegion(const TextRegion &other){
    id = other.id;
    type = other.type;
    coords = other.coords;
    lines = other.lines;
    textEquiv = other.textEquiv;
}

TextRegion& TextRegion::operator =(const TextRegion &rhs){
    id = rhs.id;
    type = rhs.type;
    coords = rhs.coords;
    lines = rhs.lines;
    textEquiv = rhs.textEquiv;
    return *this;
}

// Document

bool Document::checkEnd(QXmlStreamReader &xml, QString name){
    QXmlStreamReader::TokenType token;
    getToken(xml,token);

    if(token == QXmlStreamReader::EndElement && xml.name() == name)
        return true;
    else
        return false;
}

int Document::readDocumentAttributes(QXmlStreamReader &xml){
    QXmlStreamAttributes attributes = xml.attributes();
    if(attributes.hasAttribute("xmlns"))
        xmlns=attributes.value("xmlns").toString();
    if(attributes.hasAttribute("xmlns:xsi"))
        xmlns_xsi=attributes.value("xmlns:xsi").toString();
    if(attributes.hasAttribute("xsi:schemaLocation"))
        xsi_schemaLocation=attributes.value("xsi:schemaLocation").toString();
    if(attributes.hasAttribute("pcGtsId"))
        pcGtsId=attributes.value("pcGtsId").toString();
    return 0;
}

int Document::readPageAttributes(QXmlStreamReader &xml){
    QXmlStreamAttributes attributes = xml.attributes();
    if(attributes.hasAttribute("imageFilename"))
        imageFilename=attributes.value("imageFilename").toString();
    else // Required
        return -1;
    if(attributes.hasAttribute("imageWidth"))
        imageWidth=attributes.value("imageWidth").toString().toInt();
    else  // Required
        return -1;
    if(attributes.hasAttribute("imageHeight"))
        imageHeight=attributes.value("imageHeight").toString().toInt();
    else // Required
        return -1;

    return 0;
}

int TextRegion::readAttributes(QXmlStreamReader &xml){
    QXmlStreamAttributes attributes = xml.attributes();
    if(attributes.hasAttribute("id")){
        id=attributes.value("id").toString();
    } 
    else  // Required
        return -1;
    if(attributes.hasAttribute("type"))
        type=attributes.value("type").toString();

    return 0;
}

int TextLine::readAttributes(QXmlStreamReader &xml){
    QXmlStreamAttributes attributes = xml.attributes();
    if(attributes.hasAttribute("id"))
        id=attributes.value("id").toString();
    else  // Required
        return -1;

    return 0;
}

int Document::readMetadata(QXmlStreamReader &xml){
    QXmlStreamReader::TokenType token;
    getToken(xml, token);

    if(!(token == QXmlStreamReader::StartElement && xml.name() == "Metadata"))
        return -1;
            // <Creator>
    getToken(xml, token);
    if(token != QXmlStreamReader::StartElement || xml.name() != "Creator")
        return -1;

    creator = xml.readElementText();
            // </Creator>
            // <Created>
    getToken(xml, token);
    if(token != QXmlStreamReader::StartElement || xml.name() != "Created")
        return -1;
    created = xml.readElementText();
            // </Created>

            // <LastChange>
    getToken(xml, token);
    if(token != QXmlStreamReader::StartElement || xml.name() != "LastChange")
        return -1;
    lastChange = xml.readElementText();
    
            // </LastChange>

            // <Comments> [0 to 1]
    getToken(xml, token);

    // if(token == QXmlStreamReader::StartElement || xml.name() == "Comments"){
    //     comments = xml.readElementText();	
    //         // </Comments>
    //     if(!checkEnd(xml,"Metadata")){	  
    //         return -1;
    // 	}
    //     // </Metadata>
    // }else{  // if there are no comments we should've just read the end of Metadata
    
    //     if(token != QXmlStreamReader::EndElement || xml.name() != "Metadata")
    //         return -1;
    // }
    return 0;
}

/* Probablement lo més pràctic siga extraure d'ací el llegir el tag d'apertura i el de tancat,
   de manera que se puga gastar per a baseline i per a coords (o siga realment este metode llegiria
   els punts continguts en una CoordsType en compte de un element anomenat "Coords") */
int Document::readCoords(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token, Coords &coords, QString name){
    // Read [0 to N] Point

  // cout << name.toStdString() << endl;
  //   getToken(xml, token);
  //   cout <<"RRRRRRRRRRRRRR"<<  xml.tokenString().toStdString() << endl;
  //   if(token != QXmlStreamReader::StartElement || xml.name() != name){
  //     cerr << "DINS" << endl;
  //       return -1;
  //   }

  getToken(xml, token);
  while(!xml.atEnd() && !xml.hasError() && !(token==QXmlStreamReader::EndElement && xml.name()==name)){   
     
      if(token==QXmlStreamReader::StartElement){// && xml.name() == "Coords"){

            QXmlStreamAttributes attributes = xml.attributes();	    
	      QStringList list;	      
	      list = attributes[0].value().toString().split(QRegExp("\\s+"));
	      for (int i = 0; i < list.size(); i++) {
		int x = list[i].section(',', 0, 0).toInt();
		int y = list[i].section(',', 1, 1).toInt();
		coords.push_back(Point(x,y));		
	      }	                 
        }
	getToken(xml, token);
    }
    

   
	//}

    if(xml.hasError()){
        QMessageBox::critical(0,"XML parsing error",xml.errorString());
        return -2; // It's an error already accounted for, we don't need to show another critical message box when this
                        // method returns (which will happen if we returned -1)
    }
    // if(!(token==QXmlStreamReader::EndElement && xml.name()==name))     
    //     return -1;
    
    return 0;
}

int TextEquiv::read(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token){
   
    if(token != QXmlStreamReader::StartElement || xml.name() != "TextEquiv"){
        if((token == QXmlStreamReader::EndElement || token == QXmlStreamReader::StartElement)
             && xml.name() == "TextLine")
            return 1; // TextEquiv not present, we're reading the next line
        if(token == QXmlStreamReader::EndElement && xml.name() == "TextRegion")
            return 1; // TextEquiv not present, we've reached the end of the region
        return -1; // None of the possible cases, error
    }
    // <TextEquiv>
    Document::getToken(xml,token);
    // if(token != QXmlStreamReader::StartElement || xml.name() != "PlainText")
    //     return -1;
     if(xml.name() == "PlainText"){
       plainText = xml.readElementText();
        Document::getToken(xml,token);
     }
    // <TextEquiv>  
     if(xml.name() == "Unicode"){      
       unicode = xml.readElementText();
       Document::getToken(xml,token);
     }

    
    if(token != QXmlStreamReader::EndElement || xml.name() != "TextEquiv")
        return -1;
    // </TextEquiv>
    return 0;
}

int TextRegion::readTextLine(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token){

  //  cout <<"readTextLine xml.name()="<< xml.name().toString().toStdString() << endl;
    if(token==QXmlStreamReader::StartElement && xml.name() == "TextLine"){
        TextLine line;
        if(line.readAttributes(xml)==-1){
            return -1;
	}
        if(Document::readCoords(xml,token,line.coords,"Coords")==-1){
	  return -1;
	}
        if(Document::readCoords(xml,token,line.baseline,"Baseline")==-1){
            return -1;
	}

        Document::getToken(xml,token);

        int ret = textEquiv.read(xml,token);

	if(ret == -1)
             return -1;
        if(ret == 0) // TextEquiv present, read the next token
            Document::getToken(xml, token);
        // If it's not present we've already read the next token

        lines.push_back(line);

    }
    return 0;
}

int Document::readTextRegion(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token){
    if(token != QXmlStreamReader::StartElement || xml.name() != "TextRegion")
        return 0;
    // <TextRegion>
    TextRegion region;
    // Attributes
    if(region.readAttributes(xml)==-1)     
        return -1;
    
    // <Coords>
    if(readCoords(xml, token, region.coords,"Coords")==-1)
      return -1;
    
    
    // </Coords>

    
    // [0 to N] TextLine, unbounded
    getToken(xml, token);
    while(!xml.atEnd() && !xml.hasError() && !(token==QXmlStreamReader::EndElement && xml.name()=="TextRegion")){
	  //&& !(token==QXmlStreamReader::StartElement && xml.name()=="TextEquiv")){
      if(region.readTextLine(xml, token)==-1){
	return 1;
      }
      
      getToken(xml,token);
    }

    int ret = region.textEquiv.read(xml,token);
    // cout << "Region id " << region.id.toStdString()<< endl;
    // cout << "Region type " << region.type.toStdString()<< endl;
    // cout << "Region Coords " << endl;
    // for (int i = 0; i < region.coords.size(); i++) {
    //   cout << "\t" << region.coords[i].x << " " << region.coords[i].y << endl; ;
    // }
    
    // cout << "Region lines "<<region.lines.size() << endl;
    // //cout << "num punts line " << region.lines[i].size() << endl;
    
    // for (int i = 0; i < region.lines.size(); i++) {
    //   cout << "line "<< i << endl;
    //   for (int j = 0; j < region.lines[i].coords.size(); j++) {
    // 	cout <<region.lines[i].coords[j].x << " " << region.lines[i].coords[j].y<< endl;
    //   }
    //   cout << endl;    
    // }

    if(ret == 0) // TextEquiv present, read the next token
        getToken(xml, token);
    // If it's not present we've already read the next token
    if(xml.hasError()){
        QMessageBox::critical(mainwindow,"XML parsing error",xml.errorString());
        return -2; // It's an error already accounted for, we don't need to show another critical message box when this
                        // method returns (which will happen if we returned -1)
    }
    if(!(token==QXmlStreamReader::EndElement && xml.name()=="TextRegion"))
        return -1;
    
    // </TextRegion>
    regions.push_back(region);

    return 0;
}

Document::Document(QWidget *parent){
    mainwindow=parent;
    xmlns="";
    xmlns_xsi="";
    xsi_schemaLocation="";
    pcGtsId="";
    creator="";
    created="";
    lastChange="";
    comments="";
    imageFilename="";
    imageWidth=imageHeight=0;
}

Document::~Document(){}

Document::Document(QString filename, QWidget *parent){
    mainwindow=parent;

    state=openFile(filename);
    if(state==-1)
        QMessageBox::critical(mainwindow,"Error","Input file does not follow the XML PAGE format or has non supported elements");
}

void Document::writeDocumentAttributes(QXmlStreamWriter &xml){
    QXmlStreamAttributes attributes;
    attributes.append("xmlns","http://schema.primaresearch.org/PAGE/gts/pagecontent/2013-07-15");
    attributes.append("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
    attributes.append("xsi:schemaLocation","http://schema.primaresearch.org/PAGE/gts/pagecontent/2013-07-15 http://schema.primaresearch.org/PAGE/gts/pagecontent/2013-07-15/pagecontent.xsd");
    attributes.append("pcGtsId",pcGtsId);
    xml.writeAttributes(attributes);
}

void Document::writePageAttributes(QXmlStreamWriter &xml){
    QXmlStreamAttributes attributes;
    attributes.append("imageFilename",imageFilename);
    attributes.append("imageWidth",QString::number(imageWidth));
    attributes.append("imageHeight",QString::number(imageHeight));
    xml.writeAttributes(attributes);
}

void TextRegion::writeAttributes(QXmlStreamWriter &xml){
    QXmlStreamAttributes attributes;
    attributes.append("id",id);
    if(type!="") // Optional attribute, maybe its not set to any value
        attributes.append("type",type);
    /* Some comprobations regarding valid types may be needed, but will be made by the tool.
        If someone else is going to use this code for other purposes may want to consider this */
    xml.writeAttributes(attributes);
}

void TextLine::writeAttributes(QXmlStreamWriter &xml){
    QXmlStreamAttributes attributes;
    attributes.append("id",id);
    xml.writeAttributes(attributes);
}

void Document::writeMetadata(QXmlStreamWriter &xml){
    xml.writeStartElement("Metadata");
    xml.writeTextElement("Creator",creator);
    xml.writeTextElement("Created",created);
    lastChange = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
    xml.writeTextElement("LastChange",lastChange);
    if(comments!="")
        xml.writeTextElement("Comments",comments);
    xml.writeEndElement();
}

void Document::writeCoords(QXmlStreamWriter &xml, Coords &coords, QString name){
    // <name> (Coords or Baseline)
    xml.writeStartElement(name);
    QString values="";
    for(int i=0;i<coords.size()-1;i++){
      
      values+=QString::number(coords.at(i).x) + "," + QString::number(coords.at(i).y);
      values+=" ";
      
      //xml.writeEmptyElement("point");
      //QXmlStreamAttributes attributes;
      // attributes.append("x",QString::number(coords.at(i).x));
      // attributes.append("y",QString::number(coords.at(i).y));
      // xml.writeAttributes(attributes);
    }
   
    values+=QString::number(coords.at(coords.size()-1).x) +","+ QString::number(coords.at(coords.size()-1).y);
  
    QXmlStreamAttributes attributes;
    attributes.append("points",values);
     xml.writeAttributes(attributes);
    // </name>
    xml.writeEndElement();
}

void TextLine::write(QXmlStreamWriter &xml){
    // <TextLine>
    xml.writeStartElement("TextLine");
    writeAttributes(xml);
    Document::writeCoords(xml,coords,"Coords");
    Document::writeCoords(xml,baseline,"Baseline");
    //if(textEquiv.plainText != "" || textEquiv.unicode != "")
        textEquiv.write(xml);
    // </TextLine>
    xml.writeEndElement();  
}

void TextRegion::write(QXmlStreamWriter &xml){
    // <TextRegion>
    xml.writeStartElement("TextRegion");
    writeAttributes(xml);
    Document::writeCoords(xml,coords,"Coords");
    for(int i=0;i<lines.size();i++)
        lines[i].write(xml);
    if(textEquiv.plainText != "" || textEquiv.unicode != "")
        textEquiv.write(xml);
    // </TextRegion>
    xml.writeEndElement();
}

void TextEquiv::write(QXmlStreamWriter &xml){
    // <TextEquiv>
    xml.writeStartElement("TextEquiv");
    //xml.writeTextElement("PlainText",plainText);
    xml.writeTextElement("Unicode",unicode);
    // </TextEquiv>
    xml.writeEndElement();
}

/* Return 0 if everything went ok */
int Document::openFile(QString filename){
    QFile xmlFile(filename);
    if(!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(mainwindow,"Error","Could not open the PAGE file");
        return -1;
    }
    QXmlStreamReader xml(&xmlFile);
    // Begin parsing
    if(xml.atEnd() || xml.hasError())
        return -2;
    QXmlStreamReader::TokenType token = xml.readNext();
    if(token != QXmlStreamReader::StartDocument)
        return -3;
    // <PcGts>
    getToken(xml, token);
    if(token != QXmlStreamReader::StartElement || xml.name() != "PcGts")
        return -4;

        // Attributes
    if(readDocumentAttributes(xml) == -1)
        return -5;

        // <Metadata>
    if(readMetadata(xml) == -1)
        return -6;
        // </Metadata>
        // <Page>
    getToken(xml, token);
            // Attributes
    if(readPageAttributes(xml) == -1)
        return -7;

        // [0 to N] Regions (unbounded)
    getToken(xml, token);
    while(!xml.atEnd() && !xml.hasError() && !(token==QXmlStreamReader::EndElement && xml.name()=="Page")){            
      if(readTextRegion(xml, token)==-1)
            return -8;
      
        getToken(xml, token);
    }

    // for (int r=0; r < regions.size(); r++){
    //   cout << "Region id " << regions[r].id.toStdString()<< endl;
    //   cout << "Region type " << regions[r].type.toStdString()<< endl;
    //   cout << "Region Coords " << endl;
    //   for (int i = 0; i < regions[r].coords.size(); i++) {
    // 	cout << "\t" << regions[r].coords[i].x << " " << regions[r].coords[i].y << endl; ;
    //   }
    
    //   cout << "Region lines "<<regions[r].lines.size() << endl;
    //   //cout << "num punts line " << region.lines[i].size() << endl;
    
    //   for (int i = 0; i < regions[r].lines.size(); i++) {
    // 	cout << "line "<< i <<": " ;
    // 	for (int j = 0; j < regions[r].lines[i].coords.size(); j++) {
    // 	  cout <<regions[r].lines[i].coords[j].x << " " << regions[r].lines[i].coords[j].y<< "; ";
    // 	}
    // 	cout << endl;    
    

    
      // 	cout << "baseLine "<< i <<": " ;
      // 	for (int j = 0; j < regions[r].lines[i].coords.size(); j++) {
      // 	  cout <<regions[r].lines[i].baseline[j].x << " " << regions[r].lines[i].baseline[j].y<< "; ";
      // 	}
      // 	cout << endl;    
      // }
      
    //}
    if(xml.hasError()){
        QMessageBox::critical(mainwindow,"XML parsing error",xml.errorString());
        return -2; // It's an error already accounted for, we don't need to show another critical message box when this
                        // method returns (which will happen if we returned -1)
    }
    if(!(token==QXmlStreamReader::EndElement && xml.name()=="Page"))
        return -1;
        // </Page>
    if(!checkEnd(xml,"PcGts"))
        return -1;
    // </pcGts>
    return 0;
}

/* Return 0 if everything went ok */
int Document::saveFile(QString filename){
    QFile xmlFile(filename);
    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(mainwindow,"Error","Could not open the file for saving");
        return -1;
    }
    QXmlStreamWriter xml(&xmlFile);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0");

    // <PcGts>
    xml.writeStartElement("PcGts");
    writeDocumentAttributes(xml);

        // <Metadata>
    writeMetadata(xml);
        // </Metadata>
        // <Page>
    xml.writeStartElement("Page");
    writePageAttributes(xml);
    for(int i=0;i<regions.size();i++)
        regions[i].write(xml);

    xml.writeEndDocument(); // </Page> </PcGts>

    return 0;
}

QString Document::get_image_Filename(){
    return imageFilename;
}

int Document::get_imageHeight(){
    return imageHeight;
}

int Document::get_imageWidth(){
    return imageWidth;
}

QVector<TextRegion>* Document::get_regions(){
    return &regions;
}

int Document::get_state(){
    return state;
}

int Document::getToken(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token){
    token = xml.readNext();
    while(xml.name()==""){
        token = xml.readNext();
        if(xml.atEnd() || xml.hasError())
            return -1;
    }
    return 0;
}
