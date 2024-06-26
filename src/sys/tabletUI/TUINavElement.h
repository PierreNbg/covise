/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef CO_UI_NAV_H
#define CO_UI_NAV_H

#include "TUIElement.h"

class QWidget;

class QTabWidget;

/** Basic Container
 * This class provides basic functionality and a
 * common interface to all Container elements.<BR>
 * The functionality implemented in this class represents a container
 * which arranges its children on top of each other.
 */
class TUINavElement : public TUIElement
{
private:
public:
    TUINavElement(int id, int type, QWidget *w, int parent, QString name);
    virtual void setEnabled(bool en) override;
    virtual void setHighlighted(bool hl) override;

    /// get the Element's classname
    virtual const char *getClassName() const override;

protected:
};
#endif
