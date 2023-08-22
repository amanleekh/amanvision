#ifndef CLICKABLELABEL_CPP
#define CLICKABLELABEL_CPP

#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) 
{
    f = f;
}


ClickableLabel::~ClickableLabel() 
{
}


void ClickableLabel::mousePressEvent(QMouseEvent* event) 
{
    event = event;
    emit clicked();
}


void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    event = event;
    emit released();
}


ClickableAdvLabel::ClickableAdvLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) 
{
    f = f;
}


ClickableAdvLabel::~ClickableAdvLabel()
{
}


void ClickableAdvLabel::mousePressEvent(QMouseEvent* event)
{
    emit clicked(event);
}


void ClickableAdvLabel::mouseReleaseEvent(QMouseEvent *event)
{
    emit released(event);
}


#endif // CLICKABLELABEL_CPP
