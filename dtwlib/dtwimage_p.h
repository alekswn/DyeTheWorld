#ifndef DTWIMAGE_P_H
#define DTWIMAGE_P_H

#include "dtwimage.h"
#include <QImage>

namespace dtw {

struct DtwImagePrivate
{
public:
    DtwImagePrivate(DtwImage *q)
        : q_ptr(q)
{}

    DtwImage * q_ptr;
    QImage m_original;
    Q_DECLARE_PUBLIC(DtwImage);
};//struct DtwImagePrivate

}//namespace dtw
#endif // DTWIMAGE_P_H

