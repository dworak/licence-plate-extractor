#ifndef POWIAT_H
#define POWIAT_H

#include <QStringList>

class Powiaty
{
public:
    Powiaty();
    bool existsInTwo(const QString &label) const {return two.contains(label);}
    bool existsInThree(const QString &label) const {return three.contains(label);}

private:
    QStringList two;
    QStringList three;
};

#endif // POWIAT_H
