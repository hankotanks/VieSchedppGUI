/* 
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qtutil.h"


QList<std::tuple<int, double, double, int>> qtUtil::pointingVectors2Lists(const std::vector<VieVS::PointingVector> &pvs)
{
    QList<std::tuple<int, double, double, int>> l;

    for(const VieVS::PointingVector &p : pvs){
        l.append(std::tuple<int, double, double, int>{p.getTime(), p.getAz(), p.getEl(), p.getSrcid()});
    }

    return l;
}

QList<qtUtil::ObsData> qtUtil::getObsData(unsigned long staid, const std::vector<VieVS::Scan> &scans)
{
    QList<ObsData> list;
    for(const VieVS::Scan &scan: scans){
        auto oidx = scan.findIdxOfStationId(staid);
        if(oidx.is_initialized()){
            int idx = oidx.get();
            ObsData obs;
            obs.srcid = scan.getSourceId();
            obs.az = scan.getPointingVector(idx).getAz();
            obs.el = scan.getPointingVector(idx).getEl();
            obs.startTime = scan.getPointingVector(idx).getTime();
            obs.endTime = scan.getPointingVector(idx,VieVS::Timestamp::end).getTime();
            obs.nsta = scan.getNSta();
            list.append(obs);
        }
    }
    return list;
}

void qtUtil::worldMap(ChartView *worldmap)
{
    QChart *worldChart = new QChart();
    worldChart->setAnimationOptions(QChart::NoAnimation);

    worldChart->setAcceptHoverEvents(true);

    QFile coastF(":/plotting/coast.txt");
    if (coastF.open(QIODevice::ReadOnly)){
        QTextStream in(&coastF);

        int c = 0;
        while (!in.atEnd()){

            QLineSeries *coast = new QLineSeries(worldChart);
            coast->setColor(Qt::gray);
            coast->setName("coast");

            while(!in.atEnd()){
                QString line = in.readLine();

                if(line == "NaN,NaN"){
                    ++c;
                    worldChart->addSeries(coast);
                    break;
                }
                QStringList split = line.split(",",QString::SplitBehavior::SkipEmptyParts);
                QString lat = split[0];
                QString lon = split[1];
                coast->append(lon.toDouble(),lat.toDouble());
            }
        }
        coastF.close();
    }

    worldChart->createDefaultAxes();
    worldChart->setAcceptHoverEvents(true);
    worldChart->legend()->hide();
    worldChart->axisX()->setRange(-180,180);
    worldChart->axisY()->setRange(-90,90);
    worldChart->setAnimationOptions(QChart::NoAnimation);

    worldmap->setChart(worldChart);
    worldmap->setMinMax(-180,180,-90,90);
    worldmap->setRenderHint(QPainter::Antialiasing);
    worldmap->setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    worldmap->setBackgroundBrush(QBrush(Qt::white));
    worldmap->setMouseTracking(true);

}

std::pair<double, double> qtUtil::radec2xy(double ra, double de)
{
    ra -= pi;
    double hn = qSqrt( 1 + qCos(de)*qCos(ra/2) );

    double x = (2 * qSqrt(2) *qCos(de) *qSin(ra/2) ) / hn;
    double y = (qSqrt(2) *qSin(de) ) / hn;

    return {x,y};
}

void qtUtil::skyMap(ChartView *skymap)
{
    QChart *skyChart = new QChart();
    skyChart->setAnimationOptions(QChart::NoAnimation);

    for(int ra = 0; ra<=360; ra+=60){
        QLineSeries *ral = new QLineSeries(skyChart);
        ral->setColor(Qt::gray);
        double lambda = ra * deg2rad;

        for(int de = -90; de<=90; de+=5){
            double phi = de * deg2rad;

            auto xy = qtUtil::radec2xy(lambda, phi);

            ral->append(xy.first, xy.second);
        }
        skyChart->addSeries(ral);
    }

    for(int de = -60; de<=60; de+=30){
        QLineSeries *del = new QLineSeries(skyChart);
        del->setColor(Qt::gray);
        double phi = de * deg2rad;

        for(int ra = 0; ra<=360; ra+=5){
            double lambda = ra * deg2rad;
            auto xy = qtUtil::radec2xy(lambda, phi);

            del->append(xy.first, xy.second);
        }
        skyChart->addSeries(del);
    }

    QLineSeries *ecliptic = new QLineSeries(skyChart);
    ecliptic->setPen(QPen(QBrush(Qt::darkGreen),3,Qt::DashLine));
    double e = qDegreesToRadians(23.4);
    for(int i=-180; i<=180; i+=5){
        double l = qDegreesToRadians((double)i);
        double b = 0;
        double lambda = qAtan2(qSin(l)*qCos(e) - qTan(b)*qSin(e),qCos(l)) + pi;
//        lambda-=M_PI;
        double phi = qAsin(qSin(b)*qCos(e) + qCos(b)*qSin(e)*qSin(l));

        auto xy = qtUtil::radec2xy(lambda,phi);

        ecliptic->append(xy.first,xy.second);
    }

    skyChart->addSeries(ecliptic);

    skyChart->createDefaultAxes();
    skyChart->setAcceptHoverEvents(true);
    skyChart->legend()->hide();
    skyChart->axisX()->setRange(-2.85,2.85);
    skyChart->axisY()->setRange(-1.45,1.45);
    skyChart->axisX()->hide();
    skyChart->axisY()->hide();
    skyChart->setAnimationOptions(QChart::NoAnimation);

    skymap->setChart(skyChart);
    skymap->setMinMax(-2.85,2.85,-1.45,1.45);
    skymap->setRenderHint(QPainter::Antialiasing);
    skymap->setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    skymap->setBackgroundBrush(QBrush(Qt::white));
    skymap->setMouseTracking(true);

}

QList<QLineSeries *> qtUtil::baselineSeries(double lat1, double lon1, QString name1, double lat2, double lon2, QString name2)
{

    QList<QLineSeries *>series;
    if(lon1>lon2){
        auto tmp1 = lon1;
        lon1 = lon2;
        lon2 = tmp1;
        auto tmp2 = lat1;
        lat1 = lat2;
        lat2 = tmp2;
    }

    if(qAbs(lon2-lon1)<180){
        QLineSeries *bl = new QLineSeries();
        bl->setPen(QPen(QBrush(Qt::darkGreen),1.5,Qt::DashLine));
        bl->append(lon1,lat1);
        bl->append(lon2,lat2);
        bl->setName(QString("%1-%2").arg(name1).arg(name2));
        series.append(bl);
    }else{

        double dx = 180-qAbs(lon1)+180-qAbs(lon2);
        double dy = lat2-lat1;

        QLineSeries *bl1 = new QLineSeries();
        bl1->setPen(QPen(QBrush(Qt::darkGreen),1.5,Qt::DashLine));
        bl1->append(lon1,lat1);
        double fracx = (180-qAbs(lon1))/dx;
        double fracy = dy*fracx;
        bl1->append(-180,lat1+fracy);
        bl1->setName(QString("%1-%2_p1").arg(name1).arg(name2));

        QLineSeries *bl2 = new QLineSeries();
        bl2->setPen(QPen(QBrush(Qt::darkGreen),1.5,Qt::DashLine));
        bl2->append(lon2,lat2);
        bl2->append(180,lat2-(dy-fracy));
        bl2->setName(QString("%1-%2_p2").arg(name1).arg(name2));

        series.append(bl1);
        series.append(bl2);
    }

    return series;
}

boost::optional<std::tuple<QString,QString,QDateTime,double,QStringList,QString,QString>>  qtUtil::searchSessionCodeInMasterFile(QString code){
    QRegularExpression re_digit("\\d");
    QRegularExpressionMatchIterator i = re_digit.globalMatch(code);

    QVector<int> digits;
    QVector<int> digit_idx;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured();
        digit_idx << match.capturedStart();
        digits << word.toInt();
    }

    // check intensive:
    bool intensive = false;
    QVector<int> yearHint;
    if(code.length()-digits.length() == 1 && code.length() == 6){
        QStringList intensiveOLC;
        intensiveOLC<< "A" << "C" << "D" << "E" << "F" << "G" << "H" << "I" << "J" << "K" << "L" << "M" << "N" << "O" << "P" << "Q" << "R" << "T" << "W";
        QString firstLetter = code.at(0);
        if (intensiveOLC.indexOf( QRegularExpression(firstLetter, QRegularExpression::CaseInsensitiveOption) ) != -1){
            if(digit_idx[0] == 1 && digit_idx[1] == 2){
                yearHint << digits[0]*10 + digits[1];
            }
            intensive = true;
        }

        if ( QString::compare(code.left(4),"I01T") == 0){
            intensive = true;
            yearHint << 1;

        }
        if ( QString::compare(code.left(2),"WD") == 0 && QString::compare(code.right(1),"Q") == 0){
            intensive = true;
            yearHint << 20;
        }
        if ( QString::compare(code.left(4), "tswz", Qt::CaseInsensitive) == 0){
            intensive = true;
            yearHint << 99;
        }

    }

    // check intensive:
    bool vgos = false;
    if(!intensive){
        if( QString::compare(code.left(1),"B") == 0 && digit_idx[0] == 1){
            vgos = true;
            yearHint << 17;
        }
        if( QString::compare(code.left(1),"BB") == 0 && digit_idx[0] == 2){
            vgos = true;
            yearHint << 13;
        }
        if( QString::compare(code.left(1),"EV") == 0 && digit_idx[0] == 2){
            vgos = true;
            yearHint << 19;
        }
        if( QString::compare(code.left(1),"K") == 0 && digit_idx[0] == 1){
            vgos = true;
            yearHint << 16;
        }
        if( QString::compare(code.left(1),"KB") == 0 && digit_idx[0] == 2){
            vgos = true;
            yearHint << 16;
        }
        if( QString::compare(code.left(1),"KT") == 0 && digit_idx[0] == 2){
            vgos = true;
            yearHint << 16;
        }
        if( QString::compare(code.left(1),"MC") == 0 && digit_idx[0] == 2){
            vgos = true;
            yearHint << 17;
        }
        if( QString::compare(code.left(1),"V") == 0 && digit_idx[0] == 1){
            vgos = true;
            yearHint << 16 << 15;
        }
        if( QString::compare(code.left(1),"VGM") == 0 && digit_idx[0] == 3){
            vgos = true;
            yearHint << 16;
        }
        if( QString::compare(code.left(1),"VGP") == 0 && digit_idx[0] == 3){
            vgos = true;
            yearHint << 16;
        }
        if( QString::compare(code.left(1),"VGT") == 0 && digit_idx[0] == 3){
            vgos = true;
            yearHint << 19 << 16;
        }
        if( QString::compare(code.left(1),"VT") == 0 && digit_idx[0] == 2){
            vgos = true;
            yearHint << 19 << 18 << 17;
        }
    }



    QStringList files_sx;
    QStringList files_int;
    QStringList files_vgos;
    QDirIterator it("./AUTO_DOWNLOAD_MASTER", QStringList() << "*.txt", QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString fileName = it.next();
        if(fileName.contains("-int")){
            files_int << fileName;
        }else if(fileName.contains("-vgos")){
            files_vgos << fileName;
        }else{
            files_sx << fileName;
        }
    }


    auto comp = [ yearHint ]( const QString& lhs, const QString& rhs ){
        QString leftStr = lhs.mid(29,2);
        QString rightStr = rhs.mid(29,2);
        int leftYear = leftStr.toInt();
        int rightYear = rightStr.toInt();

        if (yearHint.contains(leftYear) && !yearHint.contains(rightYear)){
            return true;
        }
        if (!yearHint.contains(leftYear) && yearHint.contains(rightYear)){
            return false;
        }
        if (yearHint.contains(leftYear) && yearHint.contains(rightYear)){
            int idxLeft = yearHint.indexOf(leftYear);
            int idxRight = yearHint.indexOf(leftYear);
            return idxLeft < idxRight;
        }

        if(leftYear>50){
            leftYear += 1900;
        }else{
            leftYear += 2000;
        }

        if(rightYear>50){
            rightYear += 1900;
        }else{
            rightYear += 2000;
        }

        return leftYear > rightYear;

    };

    std::sort(files_int.begin(), files_int.end(), comp);
    std::sort(files_vgos.begin(), files_vgos.end(), comp);
    std::sort(files_sx.begin(), files_sx.end(), comp);


    QStringList files;
    if(intensive){
        files.append(files_int);
        files.append(files_vgos);
        files.append(files_sx);
    }else if(vgos){
        files.append(files_vgos);
        files.append(files_sx);
        files.append(files_int);
    }else{
        files.append(files_sx);
        files.append(files_int);
        files.append(files_vgos);
    }

    QStringList found;
    int year = -1;
    bool checkNext = true;
    for( const auto & file: files){
        QFile inputFile(file);

        if (inputFile.open(QIODevice::ReadOnly)){
            QTextStream in(&inputFile);
            while (!in.atEnd()) {
               QString line = in.readLine();
               if(line.isEmpty() || line.at(0) != '|'){
                   continue;
               }
               QStringList content = line.split('|');
               if( QString::compare(content[2].simplified(), code, Qt::CaseInsensitive) == 0){
                   QString yearStr = file.mid(29,2);
                   year = yearStr.toInt();

                   found = content;
                   checkNext = false;
                   break;
               }

            }
            inputFile.close();
        }
        if (!checkNext){
            break;
        }
    }
    if(checkNext){
        return boost::none;
    }
    if (year > 70){
        year += 1900;
    }else{
        year +=2000;
    }


    QString description = found[1].simplified();
    QString sessionCode = found[2].simplified();

    int doy = found[4].simplified().toInt();
    int h = found[5].simplified().left(2).toInt();
    int m = found[5].simplified().right(2).toInt();
    QDate date(year,1,1);
    date = date.addDays(doy-1);
    QTime time(h,m);
    QDateTime sessionStart(date,time);


    double dur = found[6].simplified().toDouble();
    QString stations = found[7].simplified().split("-").at(0).simplified();
    QStringList tlc;
    for (int i = 0; i<stations.length(); i+=2){
        tlc << stations.mid(i,2);
    }
    QString scheduler = found[8].simplified();
    QString correlator = found[9].simplified();

    std::tuple<QString,QString,QDateTime,double,QStringList,QString,QString> result{description, sessionCode, sessionStart, dur, tlc, scheduler, correlator};
    return result;
}


