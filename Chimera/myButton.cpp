// created by Mark O. Brown
#include "stdafx.h"
#include "myButton.h"
#include "externals.h"

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
	dc.FillSolidRect( rt, _myRGBs[ "Button-Color" ] );
	CPen pen( PS_SOLID, 0, _myRGBs["Button-Color"]);
	dc.SelectObject( pen);
	dc.SetBkColor ( _myRGBs[ "Button-Color" ] );
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
		dc.SetTextColor( _myRGBs["Text-Emph"] );
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
	if ( IsWindowEnabled( ) )
	{
		dc.FillSolidRect( subr, _myRGBs[ "Interactable-Bkgd" ] );
	}
	else
	{
		dc.FillSolidRect( subr, _myRGBs[ "Static-Bkgd" ] );
	}
	CPen pen( PS_SOLID, 0, _myRGBs["Text-Emph"] );
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
		dc.SetTextColor( _myRGBs[ "Text-Emph" ] );
		CPen pen( PS_SOLID, 0, _myRGBs[ "Text-Emph" ] );
		dc.SelectObject( pen );
		// draw the X
		dc.MoveTo( subr.TopLeft( ) );
		dc.LineTo( subr.BottomRight( ) );
		dc.MoveTo( { subr.right, subr.top } );
		dc.LineTo( { subr.left, subr.bottom } );
	}
	else
	{
		dc.SetTextColor( _myRGBs[ "Text-Emph" ] );
	}
	// Get the caption which have been set
	CString strTemp;
	GetWindowText( strTemp );
	dc.SelectObject( GetFont( ) );
	dc.SetBkColor( _myRGBs[ "Button-Color" ] );
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
