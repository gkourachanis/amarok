/***************************************************************************
 *   Copyright (C) 2004 by Michael Schulze                                 *
 *   mike.s@genion.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "playlistitem.h"

namespace itunesdb {

PlaylistItem::PlaylistItem( Q_UINT32 ipod_id)
    : ListItem( ITEMTYPE_PLAYLISTITEM)
{
    id = ipod_id;
}

PlaylistItem::PlaylistItem()
    : ListItem( ITEMTYPE_PLAYLISTITEM)
{
    id = PLAYLISTITEM_INVALID;
}

PlaylistItem::~PlaylistItem() {
}

const Q_UINT32& PlaylistItem::getID() const {
    return id;
}

void PlaylistItem::doneAddingData() {
    // do nothing here for now
}

};
