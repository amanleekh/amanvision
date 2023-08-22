
#include <math.h>
#include <QEvent>
#include "common_def.h"
#include "peilabel.h"

const double PI = 3.14159265358979323846264338327950288419717;


PieLabel::PieLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent)
{
    f = f;
}


PieLabel::~PieLabel()
{
}


int PieLabel::Init(int inCircle, int outCircle, int circleNum, QString path)
{
    if ((inCircle <= 0) || (outCircle <= 0) || (circleNum <= 0)
        || (inCircle > outCircle) || (circleNum > PIE_MAX_NUM))
    {
        AVM_ERR("PieLabel::Init faild\n");
        return 1;
    }

    m_inCircle = inCircle;
    m_outCircle = outCircle;
    m_circleNum = circleNum;

    m_isShow = 0;
    for (int i = 0; i < circleNum + PIE_EXT_NUM; i++)
    {
        m_plabelArr[i] = new QLabel(this);
        m_imgStr[i] = QString("");
        m_uid[i] = -1;
    }
    this->hide();

    // set pie style
    if (path != QString(""))
    {
        QString style = QString("PieLabel {background-image:url(")
            + path + QString(");}");
        this->setStyleSheet(style);
    }
    
    return 0;
}


int PieLabel::AddImage(int index, int uid, QString path)
{
    if ((index >= 0) || (index < GetIndexLimit()))
    {
        m_imgStr[index] = path;
        if (uid >= 0)
        {
            m_uid[index] = uid;
        }
        return 0;
    }
    return 1;
}


int PieLabel::FindIndexByUid(int uid)
{
    if (uid < 0)
    {
        return -1;
    }
    
    for (int i = 0; i < GetIndexLimit(); i++)
    {
        if (uid == m_uid[i])
        {
            return i;
        }
    }
    return -1;
}


int PieLabel::FindUidByIndex(int index)
{
    if ((index < 0) || (index >= GetIndexLimit()))
    {
        return -1;
    }
    return m_uid[index];
}


int PieLabel::TriggerIdxImage(int index)
{
    if ((index >= 0) && (index < GetIndexLimit()))
    {
        if (m_imgStr[index] != QString(""))
        {
            QString style = QString("PieLabel {background-image:url(")
                + m_imgStr[index] + QString(");}");
            this->setStyleSheet(style);
            return 0;
        }
    }   

    return 1;
}


void PieLabel::ShowHide(int isShow)
{
    if (m_isShow == isShow)
    {
        return;
    }

    m_isShow = isShow;
    if (isShow)
    {
        this->show();
    }
    else
    {
        this->hide();
    }
}


int PieLabel::GetIsShow()
{
    return m_isShow;
}


QLabel * PieLabel::GetLabel(int idx)
{
    if ((idx < 0) || (idx > m_circleNum))
    {
        return NULL;
    }
    return (m_plabelArr[idx]);
}


//Returns the index of the item
int PieLabel::indexAt(QMouseEvent* event)
{
    QPoint pos = mapFromGlobal(event->globalPos());
    double mousexdist = pos.x() - rect().center().x();
    double mouseydist = pos.y() - rect().center().y();
    double mouserad = sqrt(mousexdist * mousexdist + mouseydist * mouseydist);

    if (mouserad < m_inCircle)
    {
        if (mouseydist <= 0)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
    else if (mouserad > m_outCircle)
    {
        return -1;
    }
  
    double angle = acos(mousexdist / mouserad);
    if (mouseydist >= 0)
    {
        angle = 2*PI - angle;
    }
    double silce_angle = 2*PI / m_circleNum;
    double start_angle = silce_angle / 2.0;
    for (int i = 1; i <= m_circleNum + 1; i++)
    {
        if (angle < start_angle)
        {
            if (i != (m_circleNum + 1))
            {
                return (i+2);
            }
            else
            {
                return 3;
            }
        }
        start_angle += silce_angle;
    }

    return -1;
}


void PieLabel::mousePressEvent(QMouseEvent* event)
{
    int item = indexAt(event);
    
    // set pie style
    if ((item >= 0) && (item < GetIndexLimit()))
    {
        if (m_imgStr[item] != QString(""))
        {
            QString style = QString("PieLabel {background-image:url(")
                + m_imgStr[item] + QString(");}");
            this->setStyleSheet(style);
        }
    }
    
    emit clicked(item);
}


void PieLabel::mouseReleaseEvent(QMouseEvent *event)
{
    int item = indexAt(event);

    // set pie style
    if ((item >= 0) && (item < GetIndexLimit()))
    {
        if (m_imgStr[item] != QString(""))
        {
            QString style = QString("PieLabel {background-image:url(")
                + m_imgStr[item] + QString(");}");
            this->setStyleSheet(style);
        }
    }
 
    emit released(item);
}
