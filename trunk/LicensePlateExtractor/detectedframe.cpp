#include "detectedframe.h"
#include <QDebug>

detectedframe::detectedframe()
{

}

void detectedframe::AddPlate(detectedplate plate)
{
    bool flag=false;
    for(int i=0;i<plates.length()&&!flag;i++)
    {
        if (plates[i].match(plate))
        {
//            qDebug() << "matched";
//            for (int j=0;j<plate.characters.size();j++)
//            {
//                qDebug() << plate.characters[j].first << " " << plate.characters[j].second;

//            }
            plates[i].lastFrame = plate.lastFrame;
            plates[i].plateRect = plate.plateRect;
            if (plates[i].characters.size()==plate.characters.size())
            {
//                qDebug() << "probabilities";
                for (int j=0;j<plate.characters.size();j++)
                {
                    if (plates[i].characters[j].first!=plate.characters[j].first)
                    {
                        qDebug() << plate.characters[j].first << " " << plate.characters[j].second <<" "<< plates[i].characters[j].first << " " << plates[i].characters[j].second;
                        if (plates[i].characters[j].second<plate.characters[j].second)
                        {
                            plates[i].characters[j] = plate.characters[j];
                        }
                        //testowo
                        else
                        {
                            plates[i].characters[j].second*=0.99;
                        }
                    }
                    else if (plates[i].characters[j].second<plate.characters[j].second)
                    {
                        plates[i].characters[j].second=plate.characters[j].second;
                    }
                }

                if (plates[i].division!=plate.division)
                {
                    if (plates[i].division==-1)
                    {
                        plates[i].division = plate.division;
                    }
                    else if (plate.division!=-1)
                    {
                        if (plate.characters.size()>plates[i].characters.size() /*&& plate.getDivisionProbab()>plates[i].getDivisionProbab()-0.1*/)
                        {
                            plates[i].division = plate.division;
                        }
                    }
                }
            }
            //nierowna liczba liter
            else if (plate.characters.size()==7 || plate.characters.size()==8)
            {
                if (plates[i].characters.size()<7 || plates[i].probab< plate.probab)
                {
                plates[i].characters = plate.characters;
                plates[i].division = plate.division;
                }
            }
            else if ((plates[i].characters.size()==7 || plates[i].characters.size()==8) && plate.characters.size()!=7 && plate.characters.size()!=8)
            {}
            else if (plate.characters.size()>plates[i].characters.size())
            {
                plates[i].characters = plate.characters;
                plates[i].division = plate.division;
            }

            plates[i].computeProbab();
            flag=true;
        }
    }
    if (!flag)
    {
        qDebug() << "added" << plate.toString();
//        for (int j=0;j<plate.characters.size();j++)
//        {
//            qDebug() << plate.characters[j].first << " " << plate.characters[j].second;

//        }
        plates.append(plate);
    }
}

detectedplate detectedframe::GetPlate(cv::Rect rect, int frame)
{
    detectedplate plate(rect,frame);
    for(int i=0;i<plates.length();i++)
    {
        if (plates[i].match(plate))
        {
            return plates[i];
        }
    }
    return plate;
}

void detectedframe::clear()
{
    plates.clear();
}


