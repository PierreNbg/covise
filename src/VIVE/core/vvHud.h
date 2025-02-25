/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#pragma once
#include <util/common.h>
namespace vive
{
class VVCORE_EXPORT vvHud
{
    vvHud();
public:
    virtual ~vvHud();
    static vvHud *instance();

    void show();
    void hide();
    void redraw();
    void hideLater(float time = -1.);
    bool update();
    bool isVisible() const;

    void setText1(const std::string &text);
    void setText2(const std::string &text);
    void setText3(const std::string &text);

protected:
    bool visible;
   /* vsg::ref_ptr<osgText::Text> line1;
    vsg::ref_ptr<osgText::Text> line2;
    vsg::ref_ptr<osgText::Text> line3;
    vsg::ref_ptr<osg::Geode> geode;
    vsg::ref_ptr<osg::Camera> camera;*/

    double hudTime;
    double logoTime;
    bool doHide;

private:
    static vvHud *instance_;
};
}
