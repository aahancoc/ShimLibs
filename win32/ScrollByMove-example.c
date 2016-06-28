/******************************************************************************
 *	Sample code for a window routine with scrolling.
 *****************************************************************************/

LRESULT CALLBACK Dlg_ScrollSample(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDCANCEL:
			SendMessage(hwnd, WM_CLOSE, 0, IDCANCEL);
			break;
		case IDOK:
			SendMessage(hwnd, WM_CLOSE, 0, IDOK);
			break;
		}
	break;
	
	case WM_VSCROLL:{
		int ScrollPos, ScrollPosNew, ScrollDelta, TrackPos;
		int ViewSize, TotalSize, LineSize;
		int *LineSizePtr;
		RECT ClientRect;
		
		if(lParam != NULL){
			//Scroll message sent by child
			break;
		}
		
		//Get position, total size, and viewsize of scrollbar
		{
			SCROLLINFO SbInfo;
			SbInfo.cbSize = sizeof(SCROLLINFO);
			SbInfo.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &SbInfo);
			
			ScrollPos = SbInfo.nPos;
			ViewSize = SbInfo.nPage;
			TotalSize = SbInfo.nMax;
			TrackPos = SbInfo.nTrackPos;
		}
		
		//Get line size from storage
		LineSizePtr = GetProp(hwnd, "LineCount");
		LineSize = TotalSize / *LineSizePtr;
		
		//Get window area
		GetClientRect(hwnd, &ClientRect);
		
		switch(LOWORD(wParam)){
		case SB_TOP:
			ScrollPosNew = 0;
		case SB_BOTTOM:
			ScrollPosNew = TotalSize;
		break;
		
		case SB_PAGEUP:
			ScrollPosNew = ScrollPos - ViewSize;
		break;
		case SB_PAGEDOWN:
			ScrollPosNew = ScrollPos + ViewSize;
		break;
		
		case SB_LINEUP:
			ScrollPosNew = ScrollPos - LineSize;
		break;
		case SB_LINEDOWN:
			ScrollPosNew = ScrollPos + LineSize;
		break;
		
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			ScrollPosNew = TrackPos;
		break;
		}
		
		ScrollPosNew = min(ScrollPosNew, TotalSize);
		ScrollPosNew = max(0, ScrollPosNew); 
		
		{
			SCROLLINFO SbInfo;
			
			//Set scroll box position
			SbInfo.cbSize = sizeof(SCROLLINFO);
			SbInfo.fMask = SIF_POS;
			SbInfo.nPos = ScrollPosNew;
			SetScrollInfo(hwnd, SB_VERT, &SbInfo, TRUE);
			
			//Then get it, because Windows probably adjusted it
			GetScrollInfo(hwnd, SB_VERT, &SbInfo);
			ScrollPosNew = SbInfo.nPos;
			
			//Then redo the ScrollDelta
			ScrollDelta = (ScrollPos - ScrollPosNew);
		}
		
		//Scroll window
		EnumChildWindows(hwnd, ScrollByMove, ScrollDelta);
		
		//Redraw window
		InvalidateRect(hwnd, &ClientRect, FALSE);

	break;
	}
	
	case WM_SIZE:{
		int Width = LOWORD(lParam);
		int Height = HIWORD(lParam);
		SCROLLINFO WinScroll;
		
		//Recalculate scrollbar size
		WinScroll.cbSize = sizeof(SCROLLINFO);
		WinScroll.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
		GetScrollInfo(hwnd, SB_VERT, &WinScroll);
		
		//Move page if resizing causes page to be out of bounds
		if(
			WinScroll.nPos + Height > WinScroll.nMax && //OOB
			WinScroll.nMax > Height //Scrollbar still exists
		){
			RECT ClientRect;
			//Get change in window height
			int HeightDelta = Height - WinScroll.nPage;
			
			//Scroll window
			EnumChildWindows(hwnd, ScrollByMove, HeightDelta);
			
			//Redraw window
			GetClientRect(hwnd, &ClientRect);
			InvalidateRect(hwnd, &ClientRect, FALSE);
		}
		
		//Set scrollbar size
		WinScroll.fMask = SIF_PAGE | SIF_DISABLENOSCROLL;
		WinScroll.nPage = Height; //Size of viewport
		SetScrollInfo(hwnd, SB_VERT, &WinScroll, TRUE);
		
	break;
	}
	
	case WM_CLOSE:
		DestroyWindow(hwnd);
	break;
	
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	
	return 0;
}