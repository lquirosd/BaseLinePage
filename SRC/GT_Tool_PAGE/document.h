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
/* Disclaimer: This class gives limited support to the PAGE XML format, 
   meaning it parses and writes a subset of the format available elements and attributes. */

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <string>

// Definition of valid TextRegion types (labels)

#define T1 "paragraph"
#define T2 "heading"
#define T3 "caption"
#define T4 "header"
#define T5 "footer"
#define T6 "page-number"
#define T7 "drop-capital"
#define T8 "credit"
#define T9 "floating"
#define T10 "signature-mark"
#define T11 "catch-word"
#define T12 "marginalia"
#define T13 "footnote"
#define T14 "footnote-continued"
#define T15 "TOC-entry"

#define BASELINE_POINT 1
#define TEXTLINE_POINT 2

struct Point{
    int x;
    int y;

    Point(const Point &other);
    Point();
    Point(int nx, int ny);
    Point& operator=(const Point &rhs);
};

/*
  struct for drawing the handles (points in the middle of lines).
  */
struct Handle{
    int x;
    int y;
    Point * p1;
    Point * p2;
    Handle();
    Handle(const Handle &other);
    Handle(Point* np1, Point* np2);
    Handle & operator=(const Handle &rhs);
    void move(int nx, int ny);
    void recalculate();
};

typedef QVector<Point> Coords;

struct TextEquiv{
    QString plainText;
    QString unicode;

    TextEquiv(const TextEquiv &other);
    TextEquiv();
    TextEquiv& operator=(const TextEquiv &rhs);

    int read(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token);
    void write(QXmlStreamWriter &xml);
};

struct TextLine{
    QString id;
    Coords coords;
    Coords baseline;
    TextEquiv textEquiv;

    TextLine(const TextLine &other);
    TextLine();
    TextLine& operator=(const TextLine &rhs);
    int readAttributes(QXmlStreamReader &xml);
    void writeAttributes(QXmlStreamWriter &xml);
    void write(QXmlStreamWriter &xml);
    std::string toString(){
      std::string ret="";
      ret+="id "+id.toStdString() + "\n";
      ret+="Coords points";
      for (int i = 0; i < coords.size(); i++) {
	ret += coords[i].x + "," + coords[i].y;
	ret += " ";
      }
      ret+="\n";
      ret+="baseline points";
      for (int i = 0; i < baseline.size(); i++) {
	ret += baseline[i].x + " " + baseline[i].y;
	ret += ";";
      }

      ret+="\n transcription " + textEquiv.plainText.toStdString();


      
      return ret;
    }
};

struct TextRegion{
    QString id;
    QString type; // Equivalent to label
    Coords coords;
    QVector<TextLine> lines;
    TextEquiv textEquiv;

    TextRegion(const TextRegion &other);
    TextRegion();
    TextRegion& operator=(const TextRegion &rhs);
    int readAttributes(QXmlStreamReader &xml);
    int readTextLine(QXmlStreamReader &xml,  QXmlStreamReader::TokenType &token);
    void writeAttributes(QXmlStreamWriter &xml);
    void write(QXmlStreamWriter &xml);
};

class Document{
private:
    QWidget *mainwindow;
    QString xmlns;
    QString xmlns_xsi;
    QString xsi_schemaLocation;
    QString pcGtsId;
    // Metadata
    QString creator;
    QString created;
    QString lastChange;
    QString comments;
    // Page attributes
    QString imageFilename;
    int imageWidth;
    int imageHeight;
    // Page contents
    QVector<TextRegion> regions;

    int state; // 0 if everything is all right

    // Private methods
    int readDocumentAttributes(QXmlStreamReader &xml);
    int readPageAttributes(QXmlStreamReader &xml);
    int readMetadata(QXmlStreamReader &xml);
    int readTextRegion(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token);

    void writeDocumentAttributes(QXmlStreamWriter &xml);
    void writePageAttributes(QXmlStreamWriter &xml);
    void writeMetadata(QXmlStreamWriter &xml);

public:
    Document(QString filename, QWidget* parent = 0);
    Document(QWidget *parent = 0);
    ~Document();
    int openFile(QString filename);
    int saveFile(QString filename);
    QString get_image_Filename();
    int get_imageWidth();
    int get_imageHeight();
    QVector<TextRegion>* get_regions();

    int get_state();

    static int readCoords(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token, Coords &coords, QString name);
    static void writeCoords(QXmlStreamWriter &xml, Coords &coords, QString name);
    /* This is a small method to bypass the fact that sometimes between elements it reads "empty" tokens
       that have "" when you do xml.name() (which in my opinion makes no sense).
       It returns -1 if it reaches the end of the document to avoid being stuck forever or crashing.

        Seems the issue may be caused by endlines/tabulators in the xml document */
    static int getToken(QXmlStreamReader &xml, QXmlStreamReader::TokenType &token);
    static bool checkEnd(QXmlStreamReader &xml, QString name); // True if read </name>
};

#endif
