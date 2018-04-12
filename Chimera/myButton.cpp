#include "stdafx.h"
#include "myButton.h"

void CleanButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC dc;
	//Get device context object
	dc.Attach( lpDrawItemStruct->hDC );     
	CRect rt;
	// Get button rect
	rt = lpDrawItemStruct->rcItem;
	//Get state of the button
	UINT state = lpDrawItemStruct->itemState;  
	dc.FillSolidRect( rt, RGB( 15, 15, 45 ) );
	CPen pen( PS_SOLID, 0, RGB( 50, 50, 50) );
	dc.SelectObject( pen);
	dc.MoveTo( rt.TopLeft() );
	dc.LineTo( { rt.right, rt.top } );
	dc.LineTo( rt.BottomRight( ) );
	dc.LineTo( { rt.left, rt.bottom} );
	dc.LineTo( rt.TopLeft( ) );
	// If it is pressed...
	if ( (state & ODS_SELECTED) )
	{
		// Draw a sunken face
		//dc.DrawEdge( rt, EDGE_SUNKEN, BF_RECT );    		
		dc.SetTextColor( RGB( 255, 255, 255 ) );
	}
	else
	{
		dc.SetTextColor( RGB( 101, 123, 131 ) );
		// Draw a raised face
		//dc.DrawEdge( rt, EDGE_RAISED, BF_RECT );
	}
	// Get the caption which have been set
	CString strTemp;
	GetWindowText( strTemp );
	dc.SelectObject( GetFont( ) );
	dc.DrawText( strTemp, rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	// Draw out the caption
	// If the button is focused
	if ( (state & ODS_FOCUS) )       
	{
		// Draw a focus rect which indicates the user 
		// that the button is focused
		int iChange = 3;
		rt.top += iChange;
		rt.left += iChange;
		rt.right -= iChange;
		rt.bottom -= iChange;
		dc.DrawFocusRect( rt );
	}
	dc.Detach( );
}


void CleanCheck::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC dc;
	//Get device context object
	dc.Attach( lpDrawItemStruct->hDC );
	CRect rt;
	// Get button rect
	rt = lpDrawItemStruct->rcItem;
	CRect subr = { rt.TopLeft( ), rt.BottomRight( ) };
	subr.right = subr.left + (subr.bottom - subr.top);
	//Get state of the button
	
	UINT state = lpDrawItemStruct->itemState;
	dc.FillSolidRect( rt, RGB( 30, 30, 30) );
	dc.FillSolidRect( subr, RGB( 15, 15, 55 ) );
	CPen pen( PS_SOLID, 0, RGB( 50, 50, 50 ) );
	dc.SelectObject( pen );
	dc.MoveTo( rt.TopLeft( ) );
	dc.LineTo( { rt.right, rt.top } );
	dc.LineTo( rt.BottomRight( ) );
	dc.LineTo( { rt.left, rt.bottom } );
	dc.LineTo( rt.TopLeft( ) );
	// If it is pressed...
	//if ( (state & ODS_SELECTED) )
	if ( GetCheck( ) )
	{
		// Draw a sunken face
		//dc.DrawEdge( rt, EDGE_SUNKEN, BF_RECT );	
		dc.SetTextColor( RGB( 200, 200, 200 ) );
		CPen pen( PS_SOLID, 0, RGB( 200, 200, 200 ) );
		dc.SelectObject( pen );
		dc.MoveTo( subr.TopLeft( ) );
		dc.LineTo( subr.BottomRight( ) );
		dc.MoveTo( { subr.right, subr.top } );
		dc.LineTo( { subr.left, subr.bottom } );
	}
	else
	{
		dc.SetTextColor( RGB( 101, 123, 131 ) );
		// Draw a raised face
		//dc.DrawEdge( rt, EDGE_RAISED, BF_RECT );
	}
	// Get the caption which have been set
	CString strTemp;
	GetWindowText( strTemp );
	dc.SelectObject( GetFont( ) );
	dc.SetBkColor( RGB( 30, 30, 30 ) );
	dc.DrawText( strTemp, rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	// Draw out the caption
	// If the button is focused
	if ( (state & ODS_FOCUS) )
	{
		// Draw a focus rect which indicates the user 
		// that the button is focused
		int iChange = 3;
		rt.top += iChange;
		rt.left += iChange;
		rt.right -= iChange;
		rt.bottom -= iChange;
		dc.DrawFocusRect( rt );
	}
	dc.Detach( );
}
