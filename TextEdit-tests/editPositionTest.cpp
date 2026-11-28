#include "../src/precomp.h"
#include "../src/AbstractEditWnd.h"
#include "../src/AutoLibrary.h"
#include "CppUnitLite/CppUnitLite.h"

class PositionListener : public EditListener {
public:
   Point position;
   PositionListener() : position( -1, -1 ) {}
   virtual void onChange( void ) {}
   virtual void onViewChange( void ) {}
   virtual void onMaxText( void ) {}
   virtual void onErrSpace( void ) {}
   virtual void onPosChange( const Point& value ) { position = value; }
   virtual void onZoomChange( const int ) {}
};

class PositionFixture {
public:
   PositionListener listener;
   HWND parent;
   AbstractEditWnd *edit;

   PositionFixture() : parent( 0 ), edit( 0 ) {
      parent = CreateWindowEx( 0, _T( "STATIC" ), _T( "" ), WS_OVERLAPPEDWINDOW,
         0, 0, 300, 150, 0, 0, GetModuleHandle( 0 ), 0 );
      if ( parent ) {
         String text;
         for ( int i = 0; i < 50; ++i ) {
            text += _T( "abc\r\n" );
         }
         edit = AbstractEditWnd::create( parent, text.c_str(), &listener, false );
         MoveWindow( *edit, 0, 0, 250, 80, TRUE );
      }
   }
   ~PositionFixture() {
      delete edit;
      if ( parent ) {
         DestroyWindow( parent );
      }
   }
};

TEST( positionAfterEditMessages, editControl ) {
   AutoLibrary richEdit( _T( "RICHED20.DLL" ) );
   CHECK( (HMODULE) richEdit != 0 );
   {
      PositionFixture fixture;
      CHECK( fixture.edit != 0 );
      const int line = 20;
      const int start = (int) SendMessage( *fixture.edit, EM_LINEINDEX, line, 0 );
      CHECK( start >= 0 );

      fixture.edit->setSel( start + 1, start + 1 );
      CHECK_EQUAL( (long) line, fixture.listener.position.y );
      CHECK_EQUAL( 1L, fixture.listener.position.x );

      SendMessage( *fixture.edit, WM_CHAR, _T( 'x' ), 0 );
      CHECK_EQUAL( (long) line, fixture.listener.position.y );
      CHECK_EQUAL( 2L, fixture.listener.position.x );

      const int firstVisible = fixture.edit->getFirstVisibleLine();
      SendMessage( *fixture.edit, EM_LINESCROLL, 0, 5 );
      const Point scrolledPosition = fixture.listener.position;
      CHECK( fixture.edit->getFirstVisibleLine() > firstVisible );
      CHECK_EQUAL( (long) line, scrolledPosition.y );
      CHECK_EQUAL( 2L, scrolledPosition.x );

      const int nextStart = (int) SendMessage( *fixture.edit, EM_LINEINDEX, line + 1, 0 );
      fixture.edit->setSel( nextStart, nextStart );
      CHECK_EQUAL( (long) (line + 1), fixture.listener.position.y );
      CHECK_EQUAL( 0L, fixture.listener.position.x );

      SendMessage( *fixture.edit, WM_KEYDOWN, VK_RIGHT, 0 );
      CHECK_EQUAL( (long) (line + 1), fixture.listener.position.y );
      CHECK_EQUAL( 1L, fixture.listener.position.x );
   }
}
