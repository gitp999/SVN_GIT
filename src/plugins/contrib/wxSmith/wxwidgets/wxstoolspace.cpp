/*
* This file is part of wxSmith plugin for Code::Blocks Studio
* Copyright (C) 2006  Bartlomiej Swiecki
*
* wxSmith is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* wxSmith is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with wxSmith; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*
* $Revision$
* $Id$
* $HeadURL$
*/

#include "wxstoolspace.h"
#include "wxsparent.h"
#include "wxsitemresdata.h"
#include "wxstool.h"

namespace
{
    const int ExtraBorderSize = 7;
    const int IconSize = 32;
    const int DragBoxSize = 6;
}

BEGIN_EVENT_TABLE(wxsToolSpace,wxScrolledWindow)
    EVT_PAINT(wxsToolSpace::OnPaint)
    EVT_LEFT_DOWN(wxsToolSpace::OnMouseClick)
END_EVENT_TABLE()

wxsToolSpace::wxsToolSpace(wxWindow* Parent,wxsItemResData* Data):
    wxScrolledWindow(Parent),
    m_First(NULL),
    m_Count(0),
    m_Data(Data),
    m_Unstable(false)
{
    SetScrollbars(5,0,1,1);
    SetClientSize(wxSize(2*ExtraBorderSize+IconSize,2*ExtraBorderSize+IconSize));
    SetVirtualSize(1,1);
}

wxsToolSpace::~wxsToolSpace()
{
    while ( m_First )
    {
        Entry* Next = m_First->m_Next;
        delete m_First;
        m_First = Next;
    }
}

void wxsToolSpace::BeforePreviewChanged()
{
    m_Unstable = true;
}

void wxsToolSpace::AfterPreviewChanged()
{
    Entry* Last = NULL;

    // Clearing current content
    while ( m_First )
    {
        Entry* Next = m_First->m_Next;
        delete m_First;
        m_First = Next;
    }

    // Iterating through all children of root item searching for tools
    m_Count = 0;
    for ( int i=0; i<m_Data->GetToolsCount(); i++ )
    {
        wxsTool* Tool = m_Data->GetTool(i);

        Entry* NewEntry = new Entry;
        NewEntry->m_Tool = Tool;
        m_Count++;

        if ( Last )
        {
            Last->m_Next = NewEntry;
        }
        else
        {
            m_First = NewEntry;
        }

        Last = NewEntry;
    }

    if ( Last )
    {
        Last->m_Next = NULL;
    }

    m_Unstable = false;
    RecalculateVirtualSize();
}

void wxsToolSpace::RefreshSelection()
{
    // Only need to refresh selection since it will redraw selection properly
    Refresh();
}

void wxsToolSpace::OnPaint(wxPaintEvent& event)
{
    wxPaintDC DC(this);

    if ( m_Unstable ) return;

    DoPrepareDC(DC);
    wxSize VirtualSize = GetVirtualSize();

    DC.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    DC.SetPen(*wxBLACK);
    DC.DrawRectangle(0,0,VirtualSize.GetWidth(),VirtualSize.GetHeight());

    int i = 0;
    for ( Entry* Tool = m_First; Tool; Tool = Tool->m_Next, i++ )
    {
        wxsTool* Item = Tool->m_Tool;
        int BeginX = i*(ExtraBorderSize+IconSize) + ExtraBorderSize;
        int BeginY = ExtraBorderSize;
        int EndX = BeginX + IconSize;
        int EndY = BeginY + IconSize;
        DC.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        DC.DrawRectangle(BeginX-1,BeginY-1,EndX-BeginX+2,EndY-BeginY+2);
        DC.DrawBitmap(Item->GetInfo().Icon32,BeginX,BeginY,true);

        if ( Item->GetIsSelected() )
        {
            // Drawing fout drag boxes
            if ( m_Data->GetRootSelection() != Item )
            {
                DC.SetPen(*wxGREY_PEN);
                DC.SetBrush(*wxGREY_BRUSH);
            }
            else
            {
                DC.SetPen(*wxBLACK_PEN);
                DC.SetBrush(*wxBLACK_BRUSH);
            }

            DC.DrawRectangle(BeginX-DragBoxSize/2,BeginY-DragBoxSize/2,DragBoxSize,DragBoxSize);
            DC.DrawRectangle(BeginX-DragBoxSize/2,EndY-DragBoxSize/2,DragBoxSize,DragBoxSize);
            DC.DrawRectangle(EndX-DragBoxSize/2,BeginY-DragBoxSize/2,DragBoxSize,DragBoxSize);
            DC.DrawRectangle(EndX-DragBoxSize/2,EndY-DragBoxSize/2,DragBoxSize,DragBoxSize);
        }
    }
}

void wxsToolSpace::OnMouseClick(wxMouseEvent& event)
{
    if ( m_Unstable ) return;

    // Finding out which tool has been clicked
    // TODO: Check if mouse coordinates are yet shifted to virtual area

    if ( event.GetY() < ExtraBorderSize ) return;
    if ( event.GetY() >= ExtraBorderSize + IconSize ) return;

    int ToolNumber = event.GetX() / (ExtraBorderSize + IconSize);
    int InToolPos  = event.GetX() % (ExtraBorderSize + IconSize);

    if ( ToolNumber >= m_Count ) return;
    if ( InToolPos < ExtraBorderSize ) return;

    Entry* Tool;
    for ( Tool = m_First; Tool && ToolNumber--; Tool = Tool->m_Next );
    if ( !Tool ) return;

    if ( !Tool->m_Tool->GetIsSelected() )
    {
        m_Data->SelectItem(Tool->m_Tool,!event.ControlDown());
    }
    else
    {
        m_Data->SelectItem(Tool->m_Tool,false);
    }

}

void wxsToolSpace::RecalculateVirtualSize()
{
    SetVirtualSize(
        m_Count*(ExtraBorderSize + IconSize) + ExtraBorderSize,
        2*ExtraBorderSize + IconSize);
}
