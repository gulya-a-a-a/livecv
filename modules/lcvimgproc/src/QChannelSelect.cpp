/****************************************************************************
**
** Copyright (C) 2014 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV application.
**
** GNU General Public License Usage
** 
** This file may be used under the terms of the GNU General Public License 
** version 3.0 as published by the Free Software Foundation and appearing 
** in the file LICENSE.GPL included in the packaging of this file.  Please 
** review the following information to ensure the GNU General Public License 
** version 3.0 requirements will be met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/
#include "QChannelSelect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

QChannelSelect::QChannelSelect(QQuickItem *parent) :
    QMatFilter(parent)
{
}

QChannelSelect::~QChannelSelect(){
}

void QChannelSelect::transform(cv::Mat &in, cv::Mat &out){
    if ( out.channels() == 3 )
        cv::cvtColor(out, out, CV_BGR2GRAY);
    if ( !in.empty() ){
        if ( in.channels() == 1 ){
            in.copyTo(out);
            return;
        }
        std::vector<cv::Mat> channels(3);
        cv::split(in, channels);
        channels[m_channel].copyTo(out);
    }
}
