#include "powiaty.h"

#include <QFile>
#include <QTextStream>

#include <QDebug>

Powiaty::Powiaty()
{
    QFile file(":/powiaty/powiaty");
    file.open(QFile::ReadOnly);
    QTextStream ts(&file);
    QString line;
    while(!(line = ts.readLine()).isNull())
        if(line.size() == 2)
            two.append(line);
        else
            three.append(line);
    file.close();
}
