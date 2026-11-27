/*
 * Displays line numbers beside an edit control.
 */

#pragma once

#include "AbstractEditWnd.h"

class LineNumberWnd : public Window {
private:
   AbstractEditWnd *m_pEditWnd;

public:
   LineNumberWnd( HWND hwndParent, AbstractEditWnd *pEditWnd );
   void invalidate( void );
   virtual LRESULT dispatch( UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );
};

// end of file
