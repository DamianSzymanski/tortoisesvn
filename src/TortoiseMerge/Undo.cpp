// TortoiseMerge - a Diff/Patch program

// Copyright (C) 2006-2007, 2010-2011 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "StdAfx.h"
#include "Undo.h"

#include "BaseView.h"

void viewstate::AddViewLineFormView(CBaseView *pView, int nLine, int nViewLine, bool bAddEmptyLine)
{
    if (!pView || !pView->m_pViewData)
        return;
    difflines[nViewLine] = pView->m_pViewData->GetLine(nViewLine);
    linestates[nViewLine] = pView->m_pViewData->GetState(nViewLine);
    linesEOL[nLine] = pView->m_pViewData->GetLineEnding(nLine);
    if (bAddEmptyLine)
    {
        addedlines.push_back(nViewLine + 1);
        pView->AddEmptyLine(nLine);
    }
}

CUndo& CUndo::GetInstance()
{
    static CUndo instance;
    return instance;
}

CUndo::CUndo()
{
    m_originalstate = 0;
}

CUndo::~CUndo()
{
}

/** \note for backward compatibility only */
void CUndo::AddState(const viewstate& leftstate, const viewstate& rightstate, const viewstate& bottomstate, POINT pt)
{
    allviewstate allstate;
    allstate.bottom = bottomstate;
    allstate.right = rightstate;
    allstate.left = leftstate;
    AddState(allstate, pt);
}

void CUndo::AddState(const allviewstate& allstate, POINT pt)
{
    m_viewstates.push_back(allstate);
    m_caretpoints.push_back(pt);
}

bool CUndo::Undo(CBaseView * pLeft, CBaseView * pRight, CBaseView * pBottom)
{
    if (!CanUndo())
        return false;

    if (m_groups.size() && m_groups.back() == m_caretpoints.size())
    {
        m_groups.pop_back();
        std::list<int>::size_type b = m_groups.back();
        m_groups.pop_back();
        while (b < m_caretpoints.size())
            UndoOne(pLeft, pRight, pBottom);
    }
    else
        UndoOne(pLeft, pRight, pBottom);

    CBaseView * pActiveView = NULL;

    if (pBottom && pBottom->HasCaret())
    {
        pActiveView = pBottom;
    }
    else
    if (pRight && pRight->HasCaret())
    {
        pActiveView = pRight;
    }
    else
    //if (pLeft && pLeft->HasCaret())
    {
        pActiveView = pLeft;
    }


    if (pActiveView) {
        pActiveView->ClearSelection();
        pActiveView->BuildAllScreen2ViewVector();
        pActiveView->RecalcAllVertScrollBars();
        pActiveView->RecalcAllHorzScrollBars();
        pActiveView->EnsureCaretVisible();
        pActiveView->UpdateCaret();
        pActiveView->SetModified(m_viewstates.size() != m_originalstate);
        pActiveView->RefreshViews();
    }

    if (m_viewstates.size() < m_originalstate)
        // Can never get back to original state now
        m_originalstate = (size_t)-1;

    return true;
}

void CUndo::UndoOne(CBaseView * pLeft, CBaseView * pRight, CBaseView * pBottom)
{
    allviewstate allstate = m_viewstates.back();
    POINT pt = m_caretpoints.back();

    Undo(allstate.left, pLeft, pt);
    Undo(allstate.right, pRight, pt);
    Undo(allstate.bottom, pBottom, pt);

    m_viewstates.pop_back();
    m_caretpoints.pop_back();
}

/** \note interface kept for compatibility, can be merged with Undo(const viewstate& state, CBaseView * pView, const POINT& pt) */
void CUndo::Undo(const viewstate& state, CBaseView * pView)
{
    if (!pView)
        return;

    CViewData* viewData = pView->m_pViewData;
    if (!viewData)
        return;

    for (std::list<int>::const_iterator it = state.addedlines.begin(); it != state.addedlines.end(); ++it)
    {
        viewData->RemoveData(*it);
    }
    for (std::map<int, DWORD>::const_iterator it = state.linelines.begin(); it != state.linelines.end(); ++it)
    {
        viewData->SetLineNumber(it->first, it->second);
    }
    for (std::map<int, DWORD>::const_iterator it = state.linestates.begin(); it != state.linestates.end(); ++it)
    {
        viewData->SetState(it->first, (DiffStates)it->second);
    }
    for (std::map<int, EOL>::const_iterator it = state.linesEOL.begin(); it != state.linesEOL.end(); ++it)
    {
        viewData->SetLineEnding(it->first, (EOL)it->second);
    }
    for (std::map<int, CString>::const_iterator it = state.difflines.begin(); it != state.difflines.end(); ++it)
    {
        viewData->SetLine(it->first, it->second);
    }
    for (std::map<int, viewdata>::const_iterator it = state.removedlines.begin(); it != state.removedlines.end(); ++it)
    {
        viewData->InsertData(it->first, it->second.sLine, it->second.state, it->second.linenumber, it->second.ending, it->second.hidestate, it->second.movedIndex);
    }
}

void CUndo::Undo(const viewstate& state, CBaseView * pView, const POINT& pt)
{
    Undo(state, pView);

    if (!pView)
        return;

    if (pView->HasCaret())
    {
        pView->SetCaretPosition(pt);
        pView->EnsureCaretVisible();
    }


}

void CUndo::Clear()
{
    m_viewstates.clear();
    m_caretpoints.clear();
    m_groups.clear();
    m_originalstate = 0;
}
