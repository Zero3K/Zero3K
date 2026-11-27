#include "precomp.h"
#include "LineNumberWnd.h"
#include "Exception.h"
#include "formatNumber.h"
#include "utils.h"
#include <richedit.h>


LineNumberWnd::LineNumberWnd( HWND hwndParent, AbstractEditWnd *pEditWnd )
   : m_pEditWnd( pEditWnd )
{
   assert( IsWindow( hwndParent ) );
   assert( isGoodPtr( pEditWnd ) );

   attach( CreateWindowEx(
      0, _T( "STATIC" ), _T( "" ), WS_CHILD | WS_VISIBLE,
      0, 0, 0, 0, hwndParent, 0, getModuleHandle(), 0 ) );

   if ( !IsWindow( *this ) ) {
      throwException( _T( "error creating line number window" ) );
   }
}


void LineNumberWnd::invalidate( void ) {
   InvalidateRect( *this, 0, FALSE );
}


LRESULT LineNumberWnd::dispatch( UINT msg, WPARAM wParam, LPARAM lParam ) {
   if ( WM_PAINT == msg ) {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint( *this, &ps );
      RECT rc;
      GetClientRect( *this, &rc );
      FillRect( hdc, &rc, GetSysColorBrush( COLOR_BTNFACE ) );

      HFONT hfont = GetWindowFont( *m_pEditWnd );
      HFONT hfontOld = (HFONT) SelectObject( hdc, hfont );
      TEXTMETRIC tm;
      if ( GetTextMetrics( hdc, &tm ) ) {
         int nLineHeight = tm.tmHeight + tm.tmExternalLeading;
         int nNumerator = 0;
         int nDenominator = 0;
         if ( 0 < nLineHeight &&
              SendMessage(
                 *m_pEditWnd, EM_GETZOOM,
                 reinterpret_cast< WPARAM >( &nNumerator ),
                 reinterpret_cast< LPARAM >( &nDenominator ) ) &&
              0 < nDenominator )
         {
            nLineHeight = MulDiv( nLineHeight, nNumerator, nDenominator );
         }

         if ( 0 < nLineHeight ) {
            SetBkMode( hdc, TRANSPARENT );
            const int nFirstVisibleLine = m_pEditWnd->getFirstVisibleLine();
            RECT rcText = { 0 };
            SendMessage( *m_pEditWnd, EM_GETRECT, 0,
               reinterpret_cast< LPARAM >( &rcText ) );
            POINT ptTextOrigin = { rcText.left, rcText.top };
            MapWindowPoints( *m_pEditWnd, *this, &ptTextOrigin, 1 );
            const int nVisibleRows =
               (rc.bottom - ptTextOrigin.y + nLineHeight - 1) / nLineHeight;
            int nPreviousLine = -1;
            for ( int nRow = 0; nRow < nVisibleRows; ++nRow ) {
               const int nDisplayLine = nFirstVisibleLine + nRow;
               const int nChar = (int) SendMessage(
                  *m_pEditWnd, EM_LINEINDEX, nDisplayLine, 0 );
               if ( 0 > nChar ) {
                  break;
               }

               const int nLine = m_pEditWnd->lineFromChar( nChar );
               if ( 0 > nLine ) {
                  continue;
               }
               if ( nLine != nPreviousLine ) {
                  const String strLine = formatNumber( nLine + 1 );
                  const int y = ptTextOrigin.y + nRow * nLineHeight;
                  RECT rcLine = { 2, y, rc.right - 5, y + nLineHeight };
                  DrawText( hdc, strLine.c_str(), -1,
                     &rcLine, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX );
                  nPreviousLine = nLine;
               }
            }
         }
      }

      SelectObject( hdc, hfontOld );
      if ( 0 < rc.right ) {
         RECT rcBorder = { rc.right - 1, 0, rc.right, rc.bottom };
         FillRect( hdc, &rcBorder, GetSysColorBrush( COLOR_3DSHADOW ) );
      }
      EndPaint( *this, &ps );
      return 0;
   }

   return Window::dispatch( msg, wParam, lParam );
}

// end of file
