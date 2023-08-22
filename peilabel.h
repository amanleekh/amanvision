#ifndef PEILABEL_H
#define PEILABEL_H

#include <QLabel>
#include <QWidget>
#include <QString>
#include <QMouseEvent>


#define PIE_MAX_NUM     (8)
#define PIE_EXT_NUM     (3)


class PieLabel : public QLabel
{
    Q_OBJECT

public:
    explicit PieLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~PieLabel();

public:
    int Init(int inCircle, int outCircle, int circleNum, QString path);
    /**< uid>=0, less than 0 is not vaild */
    int AddImage(int index, int uid, QString path);
    /**< return idx, or -1 */
    int FindIndexByUid(int uid);
    int FindUidByIndex(int index);
    int TriggerIdxImage(int index);

    void ShowHide(int isShow);
    int GetIsShow();
    QLabel *GetLabel(int idx);
    int GetNum()
    {
        return m_circleNum;
    }
    int GetIndexLimit()
    {
        return (m_circleNum+PIE_EXT_NUM);
    }

signals:
    void clicked(int num);
    void released(int num);

private:
    int indexAt(QMouseEvent* event);
    
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    int m_inCircle;
    int m_outCircle;
    int m_circleNum;
    int m_isShow;
    QLabel *m_plabelArr[PIE_MAX_NUM+PIE_EXT_NUM];   /**< label text context */
    QString m_imgStr[PIE_MAX_NUM+PIE_EXT_NUM];  /**< label background image */
    int m_uid[PIE_MAX_NUM+PIE_EXT_NUM];
};


#endif // PEILABEL_H
