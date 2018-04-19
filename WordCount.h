// implementatin of this specific plug-in is here:
//

#ifndef WM_DPICHANGED_BEFOREPARENT
#define WM_DPICHANGED_BEFOREPARENT      0x02E2
#define WM_DPICHANGED_AFTERPARENT       0x02E3
#define WM_GETDPISCALEDSIZE             0x02E4
#endif

#define ZERO_INIT_FIRST_MEM(classname, firstmem)  ZeroMemory( &firstmem, sizeof( classname ) - ((char*)&firstmem - (char*)this) );

#define MAX_TITLE	256

#define TYPE_CHARS	0
#define TYPE_WIDTHS	1
#define TYPE_WORDS	2
#define TYPE_LINES	3
#define TYPE_VIEW_LINES	4
#define TYPE_PAGES	5

#define TYPE_MASK			0x00ff
#define TYPE_REGEXP			0x0100

#define MAX_ALPHANUMERIC_CHARS	260

INT_PTR CALLBACK PropDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK MainProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK CustomizeDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK CustPropDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
UINT WINAPI WorkThread(LPVOID lpData);

#define SIGNATURE_QUERY_ARRAY 0x00FE0200

LPCTSTR const szQueryArrayEntry = _T("QueryArray");
LPCTSTR const szAutoRefresh = _T("AutoRefresh");
LPCTSTR const szColumnEntryFormat = _T("ColumnWidth%d");
LPCTSTR const szAlphanumericCharsEntry = _T("AlphanumericChars");
LPCTSTR const szDefAlphanumericChars = _T("_");

__forceinline BOOL IsHighSurrogate( WCHAR ch )
{
	return (ch >= 0xd800 && ch <= 0xdbff);
}

__forceinline BOOL IsLowSurrogate( WCHAR ch )
{
	return (ch >= 0xdc00 && ch <= 0xdfff);
}

void CenterWindow( HWND hDlg )
{
	_ASSERT(::IsWindow(hDlg));
	HWND hwndParent = GetParent(hDlg);

	RECT rcDlg;
	::GetWindowRect(hDlg, &rcDlg);
	RECT rcArea;
	RECT rcCenter;

	if( hwndParent != NULL ){
		DWORD dwStyleCenter = ::GetWindowLong(hwndParent, GWL_STYLE);
		if(!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
			hwndParent = NULL;
	}

	HMONITOR hMonitor = NULL;
	if(hwndParent != NULL) {
		hMonitor = ::MonitorFromWindow(hwndParent, MONITOR_DEFAULTTONEAREST);
	}
	else {
		hMonitor = ::MonitorFromWindow(hDlg, MONITOR_DEFAULTTONEAREST);
	}
	_ASSERT(hMonitor != NULL);

	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);
	VERIFY( ::GetMonitorInfo(hMonitor, &minfo) );

	rcArea = minfo.rcWork;

	if(hwndParent == NULL)
		rcCenter = rcArea;
	else
		::GetWindowRect(hwndParent, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	if(xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;
	if(xLeft < rcArea.left)
		xLeft = rcArea.left;

	if(yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;
	if(yTop < rcArea.top)
		yTop = rcArea.top;

	::SetWindowPos(hDlg, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

class CQuery
{
public:
	int		m_nType;
	wstring m_sTitle;
	wstring m_sExp;
	UINT_PTR m_nCount;
	UINT_PTR m_nAll;
	UINT_PTR m_nSel;

	CQuery( int nType, LPCWSTR pszTitle, LPCWSTR pszExp )
	{
		m_nType = nType;
		m_sTitle = pszTitle;
		m_sExp = pszExp;
		m_nCount = 0;
		m_nAll = 0;
		m_nSel = 0;
	}

};

typedef vector<CQuery> CQueryArray;



int WaitMessageLoop( DWORD nCount, const HANDLE* pHandles, DWORD dwMilliseconds )
{ 
	bool bQuit = false;
	int nExitCode = 0;
	DWORD dwResult = 0;
    do {
		MSG msg = { 0 }; 
        while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) { 
            if (msg.message == WM_QUIT) {
				bQuit = true;
				nExitCode = (int)msg.wParam;
				break;
			}
			else {
				DispatchMessage( &msg ); 
			}
        }
        dwResult = MsgWaitForMultipleObjects( nCount, pHandles, FALSE, dwMilliseconds, QS_ALLINPUT ); 
    } while( dwResult == WAIT_OBJECT_0 + 1 );

	if( bQuit ){
		PostQuitMessage( nExitCode );
	}
	return (int)(dwResult - WAIT_OBJECT_0);
}

class CMyFrame : public CETLFrame<CMyFrame>
{
public:
	// _loc.dll in MUI sub folder?
	enum { _USE_LOC_DLL			= LOC_USE_EMEDLOC_DLL   };

	// string ID
	enum { _IDS_MENU			= IDS_WORDCOUNT_MENU_TEXT	};   // name of command, menu
	enum { _IDS_STATUS			= IDS_WORDCOUNT_MENU_TEXT	};   // description of command, status bar
	enum { _IDS_NAME			= IDS_WORDCOUNT_MENU_TEXT	};   // name of plug-in, plug-in settings dialog box
	enum { _IDS_VER				= IDS_PLUGIN_VERSION		};   // version string of plug-in, plug-in settings dialog box

	// bitmaps
	enum { _IDB_BITMAP			= IDB_BITMAP			};
	enum { _IDB_16C_24			= IDB_16C_24			};
	//enum { _IDB_256C_16_DEFAULT = IDB_256C_16_DEFAULT	};
	//enum { _IDB_256C_16_HOT		= IDB_256C_16_HOT		};
	//enum { _IDB_256C_16_BW		= IDB_256C_16_BW		};
	//enum { _IDB_256C_24_DEFAULT = IDB_256C_24_DEFAULT	};
	//enum { _IDB_256C_24_HOT		= IDB_256C_24_HOT		};
	//enum { _IDB_256C_24_BW		= IDB_256C_24_BW		};
	enum { _IDB_TRUE_16_DEFAULT = IDB_TRUE_16_DEFAULT	};
	enum { _IDB_TRUE_16_HOT		= IDB_TRUE_16_HOT		};
	enum { _IDB_TRUE_16_BW		= IDB_TRUE_16_BW		};
	enum { _IDB_TRUE_24_DEFAULT = IDB_TRUE_24_DEFAULT	};
	enum { _IDB_TRUE_24_HOT		= IDB_TRUE_24_HOT		};
	enum { _IDB_TRUE_24_BW		= IDB_TRUE_24_BW		};

	// masks
	enum { _MASK_TRUE_COLOR		= CLR_NONE				};
	//enum { _MASK_256_COLOR		= CLR_NONE          	};

	// whether to allow a file is opened in the same window group during the plug-in execution.
	enum { _ALLOW_OPEN_SAME_GROUP = TRUE				};

	// whether to allow multiple instances.
	enum { _ALLOW_MULTIPLE_INSTANCES = TRUE				};

	// supporting EmEditor newest version * 1000
	enum { _MAX_EE_VERSION		= 17206					};

	// supporting EmEditor oldest version * 1000
	enum { _MIN_EE_VERSION		= 17206					};

	// supports EmEditor Professional
	enum { _SUPPORT_EE_PRO		= TRUE					};

	// supports EmEditor Standard
	enum { _SUPPORT_EE_STD		= FALSE					};

	CQueryArray m_Array;

	// data that can be set zeros below
	HWND m_hwndList;  // first member for data that can be set zeros
	HWND m_hDlg;
	HANDLE m_hWorkThread;
	HANDLE m_hQueEvent;
	CQuery* m_pProp;
	UINT m_nClientID;
	int  m_iPos;
	int  m_iOldPos;
	int  m_nDpi;
	TCHAR m_szAlphanumericChars[MAX_ALPHANUMERIC_CHARS];
	bool m_bOpenStartup;
	bool m_bUpdateList;
	bool m_bAbortThread;
	bool m_bAutoRefresh;
	bool m_bArrayModified;
	bool m_bRefreshOnClose;

	BOOL DisableAutoComplete( HWND /* hwnd */ )
	{
		return FALSE;
	}

	BOOL UseDroppedFiles( HWND /* hwnd */ )
	{
		return FALSE;
	}

	LRESULT UserMessage( HWND /*hwnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
	{
		return 0;
	}

	void OnCommand( HWND /*hwndView*/ )
	{
		if( m_hwndList == NULL ){
			OpenCustomBar();
			m_bUpdateList = true;
		}
		else {
			CloseCustomBar();
		}
	}

	void OpenCustomBar()
	{
		//if( m_hwndList )  return;
		if( m_hDlg )  return;

		TCHAR sz[260];
		TCHAR szAppName[80];
		LoadString( EEGetLocaleInstanceHandle(), IDS_WORDCOUNT_MENU_TEXT, szAppName, _countof( szAppName ) );
		if( Editor_GetVersion( m_hWnd ) < 8000 ){
			LoadString( EEGetLocaleInstanceHandle(), IDS_INVALID_VERSION, sz, _countof( sz ) );
			MessageBox( m_hWnd, sz, szAppName, MB_OK | MB_ICONSTOP );
			return;
		}

		m_hDlg = CreateDialog( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDD_WORDCOUNT_MAIN ), m_hWnd, MainProc );

		if( m_hDlg != NULL ){
			CUSTOM_BAR_INFO cbi;
			ZeroMemory( &cbi, sizeof( cbi ) );
			cbi.cbSize = sizeof( cbi );
			cbi.hwndClient = m_hDlg;
			cbi.iPos = m_iPos;
			cbi.pszTitle = szAppName;
			m_nClientID = Editor_CustomBarOpen( m_hWnd, &cbi );

			if( !m_nClientID ){
				CustomBarClosed();
			}
			else {
				ShowWindow( m_hDlg, SW_SHOW );

				// The parent window is set to the custom bar window.
				_ASSERTE( cbi.hwndCustomBar && GetParent( m_hDlg ) == cbi.hwndCustomBar );
			}
		}
	}

	void CloseCustomBar()
	{
		if( !m_hwndList )  return;

		ResetThread();

		_ASSERTE( m_nClientID );
		BOOL bClosed = Editor_CustomBarClose( m_hWnd, m_nClientID );
		bClosed;  // avoid warning
		_ASSERTE( bClosed );
//		_ASSERTE( m_lpOldTreeProc == NULL );
		_ASSERTE( m_hwndList == NULL );
		_ASSERTE( m_hDlg == NULL );
//		UnsubclassTree( m_hwndList );
		CustomBarClosed();  //  call just in case.  should not be needed since EVENT_CUSTOM_BAR_CLOSED is called.
	}

	void CustomBarClosed()
	{
		ResetThread();

		if( m_hDlg ){
			if( IsWindow( m_hDlg ) ){
				DestroyWindow( m_hDlg );
			}
			_ASSERT( !IsWindow( m_hDlg ) );
			m_hDlg = NULL;
			m_hwndList = NULL;
			m_nClientID = 0;
		}
		//if( m_hwndList ){
		//	if( IsWindow( m_hwndList ) ) {
		//		DestroyWindow( m_hwndList );
		//	}
		//	m_hwndList = NULL;
		//}
		//m_nClientID = 0;
	}



	BOOL QueryStatus( HWND /*hwndView*/, LPBOOL pbChecked )
	{		
		*pbChecked = (m_hwndList != NULL);
		return TRUE;
	}

	void OnEvents( HWND hwndView, UINT nEvent, LPARAM lParam )
	{
		if( nEvent & EVENT_CREATE_FRAME ){
			m_bOpenStartup = !!GetProfileInt( _T("OpenStartup"), FALSE );
			m_iPos = GetProfileInt( _T("CustomBarPos"), CUSTOM_BAR_RIGHT );
			m_iOldPos = m_iPos;

			if( m_bOpenStartup ){
				OnCommand( hwndView );
			}
		}
		if( nEvent & EVENT_CLOSE_FRAME ){
			CloseCustomBar();
		}
		if( nEvent & (EVENT_CONFIG_CHANGED | EVENT_FILE_OPENED ) ) {
			if( m_hwndList ){
//				ResetThread();
				if( m_bAutoRefresh ){
					m_bUpdateList = true;
				}
			}
		}
		if( nEvent & EVENT_CUSTOM_BAR_CLOSING ){
			// this message arrives even if plug-in does not own this custom bar, so make sure it is mine.
			if( m_hwndList != NULL ){
				CUSTOM_BAR_CLOSE_INFO* pCBCI = (CUSTOM_BAR_CLOSE_INFO*)lParam;
				if( pCBCI->nID == m_nClientID ){
					SaveProfile();
					_ASSERTE( m_hwndList );
					_ASSERTE( GetParent( m_hwndList ) );
					ResetThread();
//					UnsubclassTree( m_hwndList );
				}
			}
		}
		if( nEvent & EVENT_CUSTOM_BAR_CLOSED ){
			// this message arrives even if plug-in does not own this custom bar, so make sure it is mine.
			if( m_hwndList != NULL ){  
				CUSTOM_BAR_CLOSE_INFO* pCBCI = (CUSTOM_BAR_CLOSE_INFO*)lParam;
				if( pCBCI->nID == m_nClientID ){
					_ASSERT( !IsWindow( m_hwndList ) );
					CustomBarClosed();
					m_bOpenStartup = (pCBCI->dwFlags & CLOSED_FRAME_WINDOW);
					WriteProfileInt( _T("OpenStartup"), m_bOpenStartup );
				}
			}
		}
		if( nEvent & EVENT_DOC_SEL_CHANGED ){
			if( m_hwndList ){
//				ResetThread();
				if( m_bAutoRefresh ){
					m_bUpdateList = true;
				}
			}
		}
		if( nEvent & (EVENT_CHANGE | EVENT_SEL_CHANGED) ){
			if( m_hwndList ){
				if( m_bAutoRefresh ){
					m_bUpdateList = true;
				}
			}
		}
		if( nEvent & (EVENT_DOC_SEL_CHANGED | EVENT_CONFIG_CHANGED | EVENT_FILE_OPENED) ){
			SetListColor();
		}
		if( nEvent & EVENT_IDLE ){
			OnIdle();
		}
	}

	void OnIdle()
	{
		if( m_hwndList ){
			if( m_bUpdateList ){
				m_bUpdateList = false;
				if( !m_hWorkThread ){
					m_hWorkThread = (HANDLE)_beginthreadex( NULL, 0, WorkThread, this, 0, NULL );
					_ASSERTE( m_hWorkThread );
					if( m_hWorkThread ){
						VERIFY( SetThreadPriority( m_hWorkThread, THREAD_PRIORITY_LOWEST ) );
					}
				}

				if( m_hwndList && m_iPos != m_iOldPos ){
					m_iOldPos = m_iPos;
					CloseCustomBar();
					OpenCustomBar();
				}
				SetEvent( m_hQueEvent );
			}
			//if( m_bFocusView ){
			//	Editor_ExecCommand( m_hWnd, EEID_ACTIVE_PANE );
			//}
			//m_bFocusView = false;
		}
	}

	BOOL QueryUninstall( HWND /*hDlg*/ )
	{
		return TRUE;
	}

	BOOL SetUninstall( HWND hDlg, LPTSTR pszUninstallCommand, LPTSTR pszUninstallParam )
	{
		TCHAR szProductCode[80] = { 0 };
		HKEY hKey = NULL;
		if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\EmSoft\\EmEditorPlugIns\\WordCount"), 0, KEY_READ, &hKey ) == ERROR_SUCCESS && hKey ){
			GetProfileStringReg( hKey, _T("ProductCode"), szProductCode, _countof( szProductCode ), _T("") );
			if( szProductCode[0] ){
				GetSystemDirectory( pszUninstallCommand, MAX_PATH );
				PathAppend( pszUninstallCommand, _T("msiexec.exe") );

				StringPrintf( pszUninstallParam, MAX_PATH, _T("/X%s"), szProductCode );
				RegCloseKey( hKey );
				return UNINSTALL_RUN_COMMAND;
			}
		}
		TCHAR sz[80];
		TCHAR szAppName[80];
		LoadString( EEGetLocaleInstanceHandle(), IDS_SURE_TO_UNINSTALL, sz, sizeof( sz ) / sizeof( TCHAR ) );
		LoadString( EEGetLocaleInstanceHandle(), IDS_WORDCOUNT_MENU_TEXT, szAppName, sizeof( szAppName ) / sizeof( TCHAR ) );
		if( MessageBox( hDlg, sz, szAppName, MB_YESNO | MB_ICONEXCLAMATION ) == IDYES ){
			return UNINSTALL_SIMPLE_DELETE;
		}
		return UNINSTALL_FALSE;
	}

	BOOL QueryProperties( HWND /*hDlg*/ )
	{
		return TRUE;
	}

	BOOL SetProperties( HWND hDlg )
	{
		DialogBox( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDD_PLUGIN_PROP ), hDlg, PropDlg );
		return TRUE;
	}

	BOOL PreTranslateMessage( HWND /*hwndView*/, MSG* pMsg )
	{
		HWND hwndFocus = GetFocus();
		if( hwndFocus ){
			if( m_hDlg && IsChild( m_hDlg, hwndFocus ) ){
				if( pMsg->message == WM_KEYDOWN ){
					bool bCtrl = GetKeyState( VK_CONTROL ) < 0;
					bool bShift = GetKeyState( VK_SHIFT ) < 0;
					if( !bCtrl ){
						if( pMsg->wParam == VK_ESCAPE ){
							if( !bShift ){
								Editor_ExecCommand( m_hWnd, EEID_ACTIVE_PANE );
								return TRUE;
							}
						}
						//else if( pMsg->wParam == VK_F6 ){
						//	Editor_ExecCommand( m_hWnd, bShift ? EEID_PREV_PANE : EEID_NEXT_PANE );
						//	return TRUE;
						//}
						else if( pMsg->wParam == VK_F5 && !bCtrl && !bShift ) {
							OnRefresh();
						}
					}
					else if( pMsg->wParam == 'A' && bCtrl && !bShift ) {
						OnSelectAll();
					}
					else if( pMsg->wParam == 'C' && bCtrl && !bShift ) {
						OnCopy();
					}
				}
				if( IsDialogMessage( m_hDlg, pMsg ) ){
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	CMyFrame()
	{
		ZERO_INIT_FIRST_MEM( CMyFrame, m_hwndList );
		m_iPos = m_iOldPos = CUSTOM_BAR_RIGHT;
		m_hQueEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
//		m_hMutex = CreateMutex( NULL, FALSE, NULL );
//		_ASSERTE( m_hMutex );
	}

	~CMyFrame()
	{
		ResetThread();
		if( m_hQueEvent ){
			CloseHandle( m_hQueEvent );
			m_hQueEvent = NULL;
		}
		//if( m_hMutex ){
		//	CloseHandle( m_hMutex );
		//	m_hMutex = NULL;
		//}
	}

	BOOL LoadArray()
	{
		m_Array.clear();
		BOOL bResult = FALSE;
		DWORD dwCount = GetProfileBinary( szQueryArrayEntry, NULL, 0 );
		if( dwCount ){
			char* pBuf = new char[ dwCount ];
			if( pBuf ){
				if( GetProfileBinary( szQueryArrayEntry, (LPBYTE)pBuf, dwCount ) ){
					int nMax, nLen, nType;
					char* p = pBuf;
					DWORD dwSign = *((DWORD*)p);
					p += sizeof( DWORD );
					if( dwSign == SIGNATURE_QUERY_ARRAY )	{
						nMax = *((int*)p);
						p += sizeof( int );
						for( int i = 0; i < nMax; i ++ ){
							nType = *((int*)p);
							p += sizeof( int );

							nLen = *((int*)p);
							p += sizeof( int );
							wstring sTitle( (LPCWSTR)p, nLen );
							p += nLen * sizeof(WCHAR);

							nLen = *((int*)p);
							p += sizeof( int );
							wstring sExp( (LPCWSTR)p, nLen );
							p += nLen * sizeof(WCHAR);

							CQuery query( nType, sTitle.c_str(), sExp.c_str() );
							m_Array.push_back( query );
						}
						_ASSERT( p == pBuf + dwCount );
						bResult = ( p == pBuf + dwCount );
					}
				}
				delete [] pBuf;
			}
		}
		return bResult;
	}

	void SaveArray()
	{
//		if( m_bArrayModified ){
			int nLen;
			BOOL bSuccess = FALSE;
			DWORD_PTR dwCount = sizeof( int );
			dwCount += sizeof(DWORD);
			CQueryArray::iterator it = m_Array.begin();
			int nMax = 0;
			while( it != m_Array.end() ){
				dwCount += (it->m_sTitle.length() + it->m_sExp.length()) * sizeof(WCHAR) + 3 * sizeof( int );
				nMax++;
				it++;
			}
			char *pBuf;
			pBuf = new char[dwCount];
			if( pBuf != NULL ){
				char* p = pBuf;
				*((DWORD*)p) = SIGNATURE_QUERY_ARRAY;
				p += sizeof( DWORD );
				*((int*)p) = nMax;
				p += sizeof( int );
				it = m_Array.begin();
				while( it != m_Array.end() ){
					*((int*)p) = it->m_nType;
					p += sizeof( int );

					nLen = (int)it->m_sTitle.length();
					*((int*)p) = nLen;
					p += sizeof( int );
					memcpy( p, it->m_sTitle.c_str(), nLen * sizeof(WCHAR) );
					p += nLen * sizeof(WCHAR);

					nLen = (int)it->m_sExp.length();
					*((int*)p) = nLen;
					p += sizeof( int );
					memcpy( p, it->m_sExp.c_str(), nLen * sizeof(WCHAR) );
					p += nLen * sizeof(WCHAR);

					it++;
				}
				_ASSERT( p == pBuf + dwCount );
				bSuccess = ( p == pBuf + dwCount );
				WriteProfileBinary( szQueryArrayEntry, (LPBYTE)pBuf, (UINT)dwCount, true );
				delete [] pBuf;
			}
//		}
//		else {
//			EraseEntry( szQueryArrayEntry );
//		}
	}

	void ResetArray()
	{
		ResetThread();
		m_Array.clear();
		TCHAR sz[80];
		LoadString( EEGetLocaleInstanceHandle(), IDS_CHARACTERS, sz, _countof( sz ) );
		m_Array.push_back( CQuery( TYPE_CHARS, sz, L"" ) );
		LoadString( EEGetLocaleInstanceHandle(), IDS_WIDTHS, sz, _countof( sz ) );
		m_Array.push_back( CQuery( TYPE_WIDTHS, sz, L"" ) );
		LoadString( EEGetLocaleInstanceHandle(), IDS_WORDS, sz, _countof( sz ) );
		m_Array.push_back( CQuery( TYPE_WORDS, sz, L"" ) );
		LoadString( EEGetLocaleInstanceHandle(), IDS_LINES, sz, _countof( sz ) );
		m_Array.push_back( CQuery( TYPE_LINES, sz, L"" ) );
		LoadString( EEGetLocaleInstanceHandle(), IDS_VIEW_LINES, sz, _countof( sz ) );
		m_Array.push_back( CQuery( TYPE_VIEW_LINES, sz, L"" ) );
		LoadString( EEGetLocaleInstanceHandle(), IDS_PAGES, sz, _countof( sz ) );
		m_Array.push_back( CQuery( TYPE_PAGES, sz, L"" ) );
	}

	void SaveProfile()
	{
		WriteProfileInt( szAutoRefresh, m_bAutoRefresh );
		if( m_hwndList ){
			int dpi = MyGetDpiForWindow( m_hwndList );
			for( int i = 0; i < 3; i++ ){
				int nWidth = ListView_GetColumnWidth( m_hwndList, i );
				TCHAR sz[80];
				StringPrintf( sz, _countof( sz ), szColumnEntryFormat, i );
				WriteProfileInt( sz, MulDiv( nWidth, 96, dpi ) );
			}
		}
	}

	void LoadProfile( HWND hwnd )
	{
		if( !LoadArray() ){
			ResetArray();
		}
		m_bAutoRefresh = !!GetProfileInt( szAutoRefresh, FALSE );
		VERIFY( CheckDlgButton( hwnd, IDC_AUTO_REFRESH, m_bAutoRefresh ) );

		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"Characters (without spaces)", L"\\S" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"Digits", L"\\d" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"Hiragana", L"[\\x{3041}-\\x{309e}]" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"Full-width Katakana", L"[\\x{309b}-\\x{309c}\\x{30a1}-\\x{30fe}]" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"Half-width Katakana", L"[\\x{ff61}-\\x{ff9f}]" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"CJK ideographs", L"[\\x{3400}-\\x{9fff}\\x{f900}-\\x{fa2d}]" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"CJK ideograph marks", L"[\\x{3000}-\\x{3037}]" ) );
		//m_Array.push_back( CQuery( TYPE_CHARS | TYPE_REGEXP, L"Hangul", L"[\\x{1100}-\\x{11f9}\\x{3131}-\\x{318e}\\x{ac00}-\\x{d7a3}]" ) );

		//m_Array.push_back( CQuery( TYPE_UPDATE_PERCENT, L"Updated (%)", L"" ) );

	}

	void ResetThread()
	{
		m_bUpdateList = false;
		if( m_hWorkThread ){
			m_bAbortThread = true;
			SetEvent( m_hQueEvent );
			VERIFY( SetThreadPriority( m_hWorkThread, THREAD_PRIORITY_ABOVE_NORMAL ) );
			WaitMessageLoop( 1, &m_hWorkThread, INFINITE );  // might block
			CloseHandle( m_hWorkThread );
			m_hWorkThread = NULL;
			//if( m_hwndList && IsWindow( m_hwndList ) ) {
			//	SendMessage( m_hwndList, WM_SETREDRAW, FALSE, 0 );
			//	ListView_DeleteAllItems( m_hwndList );
			//	SendMessage( m_hwndList, WM_SETREDRAW, TRUE, 0 );
			//}
			//m_OutlineArray.clear();
			m_bAbortThread = false;
		}
	}

	BOOL IsWordSpaceCase( WCHAR ch, LPCWSTR pszAlphanumericChars )
	{
		if( ch == 0 ){
			return TRUE;
		}
		if( iswalnum( ch ) ) {
			WORD wResult;
			if( !GetStringTypeW( CT_CTYPE3, &ch, 1, &wResult ) ) {
				return TRUE;
			}
			if( !(wResult & (C3_KATAKANA | C3_HIRAGANA | C3_IDEOGRAPH) ) ) {   // if not katakana, hiragana, nor kanji
				return FALSE;
			}
		}
		if( wcschr( pszAlphanumericChars, ch ) ){
			return FALSE;
		}
		return TRUE;
	}

	BOOL CountWordsChars( LPCWSTR psz, UINT_PTR& nChars, UINT_PTR& nWidths, UINT_PTR& nWords, bool bClearCount )
	{
		if( bClearCount ){
			for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
				it->m_nCount = 0;
			}
		}

		for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
			if( it->m_nType & TYPE_REGEXP ){
				if( (it->m_nType & TYPE_MASK) == TYPE_LINES ){
					MATCH_REGEX_INFO_EX MatchRegexInfo;
					ZeroMemory( &MatchRegexInfo, sizeof( MatchRegexInfo ) );
					MatchRegexInfo.cbSize = sizeof( MatchRegexInfo );
					MatchRegexInfo.nFlags = FLAG_FIND_CASE;
					MatchRegexInfo.pszRegex = it->m_sExp.c_str();
					MatchRegexInfo.pszText = psz;
					int nResult = Editor_MatchRegex( m_hWnd, &MatchRegexInfo );
					if( nResult == -1 ){
						return FALSE;
					}
					if( nResult ){
						it->m_nCount++;
					}
				}
			}
		}

		nWidths += (UINT_PTR)Editor_IsCharHalfOrFull( m_hWnd, psz, (INT_PTR)wcslen( psz ) );

		BOOL bAlphaNum = FALSE;
		LPCWSTR p = psz;
		LPCWSTR pWord = psz;
		LPCWSTR pEnd = NULL;
		while( *p ){
			//BOOL b = iswalnum( *p );
			BOOL b = !IsWordSpaceCase( *p, m_szAlphanumericChars );
			if( b && !bAlphaNum ){
				nWords++;
				pWord = p;
			}
			if( !b && bAlphaNum ){
				pEnd = p;
			}
			bAlphaNum = b;

			//int nCharWidth = Editor_IsCharHalfOrFull( m_hWnd, *p );
			//nWidths += nCharWidth;
			WCHAR szChar[3];
			szChar[0] = *p;
			szChar[1] = 0;
			if( IsHighSurrogate( *p++ ) ){
				if( IsLowSurrogate( *p ) ){
					szChar[1] = *p;
					szChar[2] = 0;
					p++;
				}
			}
			nChars++;

			if( pEnd ){
				LPWSTR pTemp = NULL;
				for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
					if( it->m_nType & TYPE_REGEXP ){
						if( (it->m_nType & TYPE_MASK) == TYPE_WORDS ){
							if( pTemp == NULL ){
								pTemp = new WCHAR[ pEnd - pWord + 1 ];
								if( !pTemp ){
									return FALSE;
								}
								StringCopyN( pTemp, pEnd - pWord + 1, pWord, pEnd - pWord );
							}
							MATCH_REGEX_INFO_EX MatchRegexInfo;
							ZeroMemory( &MatchRegexInfo, sizeof( MatchRegexInfo ) );
							MatchRegexInfo.cbSize = sizeof( MatchRegexInfo );
							MatchRegexInfo.nFlags = FLAG_FIND_CASE;
							MatchRegexInfo.pszRegex = it->m_sExp.c_str();
							MatchRegexInfo.pszText = pTemp;
							int nResult = Editor_MatchRegex( m_hWnd, &MatchRegexInfo );
							if( nResult == -1 ){
								return FALSE;
							}
							if( nResult ){
								it->m_nCount++;
							}
						}
					}
				}
				if( pTemp ){
					delete [] pTemp;
				}
				pEnd = NULL;
			}

			for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
				if( it->m_nType & TYPE_REGEXP ){
					if( (it->m_nType & TYPE_MASK) == TYPE_CHARS || (it->m_nType & TYPE_MASK) == TYPE_WIDTHS ){
						MATCH_REGEX_INFO_EX MatchRegexInfo;
						ZeroMemory( &MatchRegexInfo, sizeof( MatchRegexInfo ) );
						MatchRegexInfo.cbSize = sizeof( MatchRegexInfo );
						MatchRegexInfo.nFlags = FLAG_FIND_CASE;
						MatchRegexInfo.pszRegex = it->m_sExp.c_str();
						MatchRegexInfo.pszText = szChar;
						int nResult = Editor_MatchRegex( m_hWnd, &MatchRegexInfo );
						if( nResult == -1 ){
							return FALSE;
						}
						if( nResult ){
							if( (it->m_nType & TYPE_MASK) == TYPE_CHARS ){
								it->m_nCount++;
							}
							else if( (it->m_nType & TYPE_MASK) == TYPE_WIDTHS ){
								INT_PTR nCharWidth = Editor_IsCharHalfOrFull( m_hWnd, szChar, (INT_PTR)wcslen( szChar ) );
								it->m_nCount += nCharWidth;
							}
						}
					}
				}
			}
		}
		pEnd = p;
		if( bAlphaNum ){
			LPWSTR pTemp = NULL;
			for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
				if( it->m_nType & TYPE_REGEXP ){
					if( (it->m_nType & TYPE_MASK) == TYPE_WORDS ){
						if( pTemp == NULL ){
							pTemp = new WCHAR[ pEnd - pWord + 1 ];
							if( !pTemp ){
								return FALSE;
							}
							StringCopyN( pTemp, pEnd - pWord + 1, pWord, pEnd - pWord );
						}
						MATCH_REGEX_INFO_EX MatchRegexInfo;
						ZeroMemory( &MatchRegexInfo, sizeof( MatchRegexInfo ) );
						MatchRegexInfo.cbSize = sizeof( MatchRegexInfo );
						MatchRegexInfo.nFlags = FLAG_FIND_CASE;
						MatchRegexInfo.pszRegex = it->m_sExp.c_str();
						MatchRegexInfo.pszText = pTemp;
						int nResult = Editor_MatchRegex( m_hWnd, &MatchRegexInfo );
						if( nResult == -1 ){
							return FALSE;
						}
						if( nResult ){
							it->m_nCount++;
						}
					}
				}
			}
			if( pTemp ){
				delete [] pTemp;
			}
		}
		return TRUE;
	}
	
	BOOL CountBasic( bool bSel, POINT_PTR& ptTop, POINT_PTR& ptBottom, POINT_PTR& ptTopView, POINT_PTR& ptBottomView )
	{
		UINT_PTR nCharsAll = 0;
		UINT_PTR nWidthsAll = 0;
		UINT_PTR nWordsAll = 0;
		UINT_PTR nLinesAll = 0;
		UINT_PTR nCharsSel = 0;
		UINT_PTR nWidthsSel = 0;
		UINT_PTR nWordsSel = 0;
		UINT_PTR nLinesSel = 0;
		UINT_PTR nViewLinesAll = 0;
		UINT_PTR nViewLinesSel = 0;
		int nPagesAll = -1;
		int nPagesSel = -1;

		nCharsAll = 0;
		nWordsAll = 0;
		int nBeforeAfterSel = -1;

		//TCHAR sz[MAX_ALPHANUMERIC_CHARS];
		GetProfileString( EEREG_COMMON, NULL, szAlphanumericCharsEntry, m_szAlphanumericChars, _countof( m_szAlphanumericChars ), szDefAlphanumericChars );

		nLinesAll = Editor_GetLines( m_hWnd, POS_LOGICAL_W );
		nViewLinesAll = Editor_GetLines( m_hWnd, POS_VIEW );
		UINT_PTR cchBuf = 0;
		LPWSTR pBuf = NULL;
		for( UINT_PTR i = 0; i < nLinesAll; i++ ){
			if( m_bAbortThread || m_bUpdateList ){
				delete [] pBuf;
				return FALSE;
			}
			if( i % 100 == 0 ){
				WriteStatus( i * 100 / nLinesAll );
			}
			GET_LINE_INFO gli = { 0 };
			gli.flags = FLAG_LOGICAL | FLAG_WITH_CRLF;
			gli.yLine = i;
			gli.cch = 0;
			UINT_PTR cchNeeded = Editor_GetLineW( m_hWnd, &gli, NULL );
			if( cchBuf < cchNeeded ){
				delete [] pBuf;
				pBuf = new WCHAR[ cchNeeded ];
				if( !pBuf ){
					return FALSE;
				}
				cchBuf = cchNeeded;
			}
			gli.cch = cchBuf;
			*pBuf = 0;
			Editor_GetLineW( m_hWnd, &gli, pBuf );

			UINT_PTR nW = 0;
			UINT_PTR nC = 0;
			UINT_PTR nWt = 0;
			if( !CountWordsChars( pBuf, nC, nWt, nW, true ) ){
				return FALSE;
			}
			nWordsAll += nW;
			nCharsAll += nC;
			nWidthsAll += nWt;

			for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
				if( it->m_nType & TYPE_REGEXP ){
//					if( (it->m_nType & TYPE_MASK) == TYPE_CHARS ){
						it->m_nAll += it->m_nCount;
//					}
				}
			}

			if( bSel ){
				bool bCount = false;
				if( nBeforeAfterSel < 0 ){
					if( ptTop.y == (INT_PTR)i ){
						if( ptBottom.y > ptTop.y ){
							if( ptTop.x < lstrlen( pBuf ) ){
								CountWordsChars( pBuf + ptTop.x, nCharsSel, nWidthsSel, nWordsSel, true );
								bCount = true;
							}
							nBeforeAfterSel = 0;
						}
						else {
							_ASSERT( ptBottom.y == ptTop.y );
							if( ptTop.x < lstrlen( pBuf ) && lstrlen( pBuf ) >= ptBottom.x ){
								*(pBuf + ptBottom.x) = 0;
								CountWordsChars( pBuf + ptTop.x, nCharsSel, nWidthsSel, nWordsSel, true );
								bCount = true;
							}
							nBeforeAfterSel = 1;
						}
					}
				}
				else if( nBeforeAfterSel == 0 ){
					if( ptBottom.y == (INT_PTR)i ){
						if( lstrlen( pBuf ) >= ptBottom.x ){
							*(pBuf + ptBottom.x) = 0;
							CountWordsChars( pBuf, nCharsSel, nWidthsSel, nWordsSel, true );
							bCount = true;
						}
						nBeforeAfterSel = 1;
					}
					else {
						nCharsSel += nC;
						nWidthsSel += nWt;
						nWordsSel += nW;
						bCount = true;
					}
				}

				if( bCount ){
					for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
						if( it->m_nType & TYPE_REGEXP ){
//							if( (it->m_nType & TYPE_MASK) == TYPE_CHARS ){
								it->m_nSel += it->m_nCount;
//							}
						}
					}
				}
			}
		}
		delete [] pBuf;
		
		if( bSel ){
			nLinesSel = ptBottom.y - ptTop.y + 1;
			//nViewLinesSel = ptBottomView.y - ptTopView.y + 1;
			nViewLinesSel =	ptBottomView.y - ptTopView.y;
			if( ptBottomView.x > 0 ) {
				nViewLinesSel++;
			}
		}

		for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
			if( !(it->m_nType & TYPE_REGEXP) ){
				switch( it->m_nType & TYPE_MASK ){
				case TYPE_CHARS:
					it->m_nAll = nCharsAll;
					it->m_nSel = nCharsSel;
					break;
				case TYPE_WIDTHS:
					it->m_nAll = nWidthsAll;
					it->m_nSel = nWidthsSel;
					break;
				case TYPE_WORDS:
					it->m_nAll = nWordsAll;
					it->m_nSel = nWordsSel;
					break;
				case TYPE_LINES:
					it->m_nAll = nLinesAll;
					it->m_nSel = nLinesSel;
					break;
				case TYPE_VIEW_LINES:
					it->m_nAll = nViewLinesAll;
					it->m_nSel = nViewLinesSel;
					break;
				case TYPE_PAGES:
					if( nPagesAll == -1 ) {
						nPagesAll = (int)Editor_Info( m_hWnd, EI_GET_PRINT_PAGES, FALSE );
					}
					if( bSel && nPagesSel == -1 ) {
						nPagesSel = (int)Editor_Info( m_hWnd, EI_GET_PRINT_PAGES, TRUE );
					}
					it->m_nAll = nPagesAll;
					it->m_nSel = nPagesSel;
				}
			}
		}
		return TRUE;
	}

	void ResetList( HWND hwndList )
	{
		ListView_DeleteAllItems( hwndList );
		int i = 0;
		for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
			LVITEM item = { 0 };
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.pszText = (LPWSTR)it->m_sTitle.c_str();
			ListView_InsertItem( hwndList, &item );
			i++;
		}
		int nCount = ListView_GetItemCount( hwndList );
		nCount; // avoid warning
		_ASSERT( (int)m_Array.size() == nCount );
	}

	void RefreshList()
	{
		int nFlags = (Editor_GetSelType( m_hWnd ) & SEL_TYPE_MASK);
		bool bSel = (nFlags == SEL_TYPE_CHAR) || (nFlags == SEL_TYPE_LINE);
		POINT_PTR ptTop;
		POINT_PTR ptBottom;
		POINT_PTR ptTopView;
		POINT_PTR ptBottomView;
		if( bSel ){
			POINT_PTR ptStart;
			POINT_PTR ptEnd;
			Editor_GetSelStart( m_hWnd, POS_LOGICAL_W, &ptStart );
			Editor_GetSelEnd( m_hWnd, POS_LOGICAL_W, &ptEnd );
			if( ptEnd.y > ptStart.y || (ptEnd.y == ptStart.y && ptEnd.x > ptStart.x) ){
				ptBottom = ptEnd;
				ptTop = ptStart;
			}
			else {
				ptBottom = ptStart;
				ptTop = ptEnd;
			}
			Editor_GetSelStart( m_hWnd, POS_VIEW, &ptStart );
			Editor_GetSelEnd( m_hWnd, POS_VIEW, &ptEnd );
			if( ptEnd.y > ptStart.y || (ptEnd.y == ptStart.y && ptEnd.x > ptStart.x) ) {
				ptBottomView = ptEnd;
				ptTopView = ptStart;
			}
			else {
				ptBottomView = ptStart;
				ptTopView = ptEnd;
			}
		}
		else {

		}

		if( !CountBasic( bSel, ptTop, ptBottom, ptTopView, ptBottomView ) ){
			return;
		}

		_ASSERT( (int)m_Array.size() == ListView_GetItemCount( m_hwndList ) );

		int i = 0;
		for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
			LVITEM item = { 0 };
			item.mask = LVIF_TEXT;
			item.iItem = i;

			TCHAR sz[80];
			item.pszText = sz;
			item.iSubItem = 1;
			StringPrintf( sz, _countof( sz ), _T("%Iu"), it->m_nAll );
			ListView_SetItem( m_hwndList, &item );
			item.iSubItem = 2;
			if( bSel ){
				StringPrintf( sz, _countof( sz ), _T("%Iu"), it->m_nSel );
			}
			else {
				sz[0] = 0;
			}
			ListView_SetItem( m_hwndList, &item );
			it->m_nAll = 0;
			it->m_nSel = 0;
			i++;
		}
		WriteStatus( 100 );
	}

	void WriteStatus( UINT_PTR nPercent )
	{
		if( !m_hDlg )  return;
		HWND hwndPB = GetDlgItem( m_hDlg, IDC_PROGRESS );
		SendMessage( hwndPB, PBM_SETPOS, (WPARAM)nPercent, 0 );

		//int i = 0;
		//for( CQueryArray::iterator it = m_Array.begin(); it != m_Array.end(); it++ ){
		//	if( (it->m_nType & TYPE_MASK) == TYPE_UPDATE_PERCENT ){
		//		LVITEM item = { 0 };
		//		item.mask = LVIF_TEXT;
		//		item.iItem = i;

		//		TCHAR sz[80];
		//		item.pszText = sz;
		//		item.iSubItem = 1;
		//		StringPrintf( sz, _countof( sz ), _T("%Iu"), nPercent );
		//		ListView_SetItem( m_hwndList, &item );
		//	}
		//	i++;
		//}
	}

// dialog messages
	BOOL OnInitDialog( HWND hwnd )
	{
		HWND hwndList = GetDlgItem( hwnd, IDC_LIST );
		if( !hwndList )  return FALSE;
		m_hwndList = hwndList;
		_ASSERT( hwndList );

		int dpi = MyGetDpiForWindow( hwndList );
		m_nDpi = dpi;
		int anWidth[3];
		for( int i = 0; i < 3; i++ ){
			TCHAR sz[80];
			StringPrintf( sz, _countof( sz ), szColumnEntryFormat, i );
			anWidth[i] = max( GetProfileInt( sz, 80 ), 8 );
			anWidth[i] = MulDiv( anWidth[i], dpi, 96 );
		}

		TCHAR szText[32];
		LV_COLUMN lvC;
		ZeroMemory( &lvC, sizeof(lvC) );
		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;   // left-align column
		lvC.pszText = szText;
		lvC.cx = anWidth[0];
		LoadString( EEGetLocaleInstanceHandle(), IDS_QUERY, szText, _countof( szText ) );
		VERIFY( ListView_InsertColumn( hwndList, 0, &lvC ) != -1 );

		lvC.cx = anWidth[1];
		LoadString( EEGetLocaleInstanceHandle(), IDS_DOCUMENT, szText, _countof( szText ) );
		VERIFY( ListView_InsertColumn( hwndList, 1, &lvC ) != -1 );

		lvC.cx = anWidth[2];
		LoadString( EEGetLocaleInstanceHandle(), IDS_SELECTION, szText, _countof( szText ) );
		VERIFY( ListView_InsertColumn( hwndList, 2, &lvC ) != -1 );

		//InsertColumnToListView( hwndList, 0, 120, IDS_DOCUMENT, 120, IDS_SELECTION, 80, 0, 0 );
		LoadProfile( hwnd );
		ResetList( hwndList );

		HWND hwndPB = GetDlgItem( hwnd, IDC_PROGRESS );
		SendMessage( hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 100) ); 
		SendMessage( hwndPB, PBM_SETSTEP, (WPARAM) 1, 0 );

		SetListColor();

		m_bUpdateList = true;

		DWORD dwFlags = LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP;
		ListView_SetExtendedListViewStyleEx( hwndList, dwFlags, dwFlags );
		if( IsWindowVisible( m_hWnd ) ){
			return TRUE;
		}
		return FALSE;  // return FALSE to prevent focus
	}

	void OnDpiChangedAfter()
	{
		_ASSERT( m_nDpi != 0 );
		if( m_nDpi != 0 ) {
			int nNewDpi = MyGetDpiForWindow( m_hWnd );
			//if( nNewDpi > m_nDpi ) {
				for( WORD wCol = 0; wCol < 3; wCol++ ) {
					int nWidth = ListView_GetColumnWidth( m_hwndList, wCol );
					ListView_SetColumnWidth( m_hwndList, wCol, MulDiv( nWidth, nNewDpi, m_nDpi ) );
				}
			//}
			m_nDpi = nNewDpi;
		}
	}

	void OnAutoRefresh()
	{
		m_bAutoRefresh = !!IsDlgButtonChecked( m_hDlg, IDC_AUTO_REFRESH );
		if( m_bAutoRefresh ){
			m_bUpdateList = true;
		}
	}

	void OnRefresh()
	{
		m_bUpdateList = true;
		OnIdle();
	}

	void OnCustomize( HWND hwnd )
	{
		if( DialogBox( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDD_WORDCOUNT_CUSTOMIZE ), hwnd, CustomizeDlg ) == IDOK ){
		}
	}

	void OnCopy()
	{
		//HWND hwndList = GetDlgItem( m_hDlg, IDC_LIST );
		if( m_hwndList && ListView_GetSelectedCount( m_hwndList ) > 0 ){
			if( ::OpenClipboard( m_hDlg ) ){
				EmptyClipboard();

				wstring sLines;
				TCHAR szText[260];
				LVITEM item;
				ZeroMemory( &item, sizeof( item ) );
				item.pszText = szText;
				item.cchTextMax = _countof( szText );

				item.iItem = -1;
				for( ;; ) {
					item.iItem = ListView_GetNextItem( m_hwndList, item.iItem, LVNI_SELECTED );
					if( item.iItem < 0 ){
						break;
					}
					for( item.iSubItem = 0; item.iSubItem < 3; item.iSubItem++ ){
						SendMessage( m_hwndList, LVM_GETITEMTEXT, item.iItem, (LPARAM)&item );
						sLines += szText;
						if( item.iSubItem != 2 ){
							sLines += _T('\t');
						}
					}
					sLines += _T("\r\n");
				}

				size_t cchLines = sLines.length() + 1;
				HGLOBAL hData = GlobalAlloc( GMEM_MOVEABLE, cchLines * sizeof( TCHAR ) );
				if( hData != NULL ) {
					LPTSTR hpData;
					if( (hpData = (LPTSTR)GlobalLock(hData)) != NULL ) {
						StringCopy( hpData, cchLines, sLines.c_str() );
						GlobalUnlock(hData);
					}
					SetClipboardData( CF_UNICODETEXT, hData );
				}
				CloseClipboard();
			}
		}
	}

	void OnSelectAll()
	{
		//HWND hwndList = GetDlgItem( m_hDlg, IDC_LIST );
		if( m_hwndList ){
			ListView_SetItemState( m_hwndList, -1, LVIS_SELECTED, LVIS_SELECTED );
		}
	}

	void OnDlgCommand( HWND hwnd, WORD wID )
	{
		switch( wID ){
		case IDC_AUTO_REFRESH:
			OnAutoRefresh();
			break;
		case IDC_REFRESH:
			OnRefresh();
			break;
		case IDC_CUSTOMIZE:
			OnCustomize( hwnd );
			break;
		}
	}


	void CustomizeRefreshList( HWND hDlg, int iSel )
	{
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;
		ListView_DeleteAllItems( hwndList );
		for( int i = 0; i < (int)m_Array.size(); i++ ) {
			LV_ITEM item;
			ZeroMemory( &item, sizeof(item) );
			item.mask = LVIF_TEXT;
			item.iItem = i+1;
			item.pszText = LPSTR_TEXTCALLBACK;
			ListView_InsertItem( hwndList, &item );
		}
		ListView_SetItemState( hwndList, iSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, iSel, TRUE );
	}

	void OnCustomizeProp( HWND hDlg )
	{
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;
		int iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
		if( iItem >= 0 ){
			m_pProp = &m_Array[ iItem ];
			if( DialogBox( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDD_WORDCOUNT_PROP ), hDlg, CustPropDlg ) == IDOK ){
				CustomizeRefreshList( hDlg, iItem );
//				AddButtons( m_hwndToolbar );
				_ASSERT( m_hwndList );
				ResetList( m_hwndList );
				m_bArrayModified = true;
			}
		}
	}

	void OnCustomizeNew( HWND hDlg )
	{
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;
		int iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
		CQuery query( TYPE_CHARS, L"", L"" );
		m_pProp = &query;
		if( DialogBox( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDD_WORDCOUNT_PROP ), hDlg, CustPropDlg ) == IDOK ){
			if( iItem >= 0 ){
				m_Array.insert( m_Array.begin() + iItem, query );
			}
			else {
				m_Array.push_back( query );
			}

			CustomizeRefreshList( hDlg, iItem );
			_ASSERT( m_hwndList );
			ResetList( m_hwndList );
			m_bArrayModified = true;
		}
	}

	void OnCustomizeDelete( HWND hDlg )
	{
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;
		int iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
		if( iItem >= 0 ){
			m_Array.erase( m_Array.begin() + iItem );
			if( iItem == (int)m_Array.size() ){
				iItem--;
			}
			CustomizeRefreshList( hDlg, iItem );
			ResetList( m_hwndList );
			m_bArrayModified = true;
		}
	}

	void OnCustomizeCopy( HWND hDlg )
	{
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;
		int iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );

		CQuery query = m_Array[ iItem ];

		m_pProp = &query;
		if( DialogBox( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDD_WORDCOUNT_PROP ), hDlg, CustPropDlg ) == IDOK ){
			if( iItem >= 0 ){
				m_Array.insert( m_Array.begin() + iItem + 1, query );
			}
			else {
				m_Array.push_back( query );
			}

			CustomizeRefreshList( hDlg, iItem + 1 );
			ResetList( m_hwndList );
			m_bArrayModified = true;
		}
	}

	void OnCustomizeUpDown( HWND hDlg, int nDir )
	{
		_ASSERT( nDir == 1 || nDir == -1 );
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;
		int iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );

		int iNextItem = iItem + nDir;
		if( iNextItem < 0 || iNextItem >= (int)m_Array.size() ){
			return;
		}
		CQuery query = m_Array[ iItem ];
		m_Array[ iItem ] = m_Array[ iNextItem ];
		m_Array[ iNextItem ] = query;
		CustomizeRefreshList( hDlg, iNextItem );
		ResetList( m_hwndList );
		m_bArrayModified = true;
	}

	void OnCustomizeInitDialog( HWND hDlg )
	{
		CenterWindow( hDlg );
		HWND hwndList = GetDlgItem( hDlg, IDC_LIST );
		if( !hwndList )  return;

		//ListView_SetExtendedListViewStyleEx( hwndList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );
		ListView_SetExtendedListViewStyleEx( hwndList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_AUTOSIZECOLUMNS, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_AUTOSIZECOLUMNS );

		TCHAR sz[80];
		LV_COLUMN lvC = { 0 };
		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.pszText = sz;
		RECT rc;
		GetWindowRect( hwndList, &rc );
		lvC.cx = rc.right - rc.left - GetSystemMetrics( SM_CXVSCROLL ) - GetSystemMetrics( SM_CXEDGE ) * 2;
		GetWindowText( hDlg, sz, _countof( sz ) );
		VERIFY( ListView_InsertColumn( hwndList, 0, &lvC ) != -1 );

		CustomizeRefreshList( hDlg, 0 );
		m_bRefreshOnClose = false;

	}

	void OnCustomizeCommand( HWND hDlg, WPARAM wParam )
	{
		if( wParam == IDCANCEL ){
			EndDialog( hDlg, IDCANCEL );
			if( m_bArrayModified ){
				SaveArray();
			}
			if( m_bArrayModified || m_bRefreshOnClose ){
				if( m_bAutoRefresh ){
					m_bUpdateList = true;
					OnIdle();
				}
			}
		}
		else if( wParam == IDC_PROPERTIES ){
			OnCustomizeProp( hDlg );
		}
		else if( wParam == IDC_NEW ){
			OnCustomizeNew( hDlg );
		}
		else if( wParam == IDC_COPY ){
			OnCustomizeCopy( hDlg );
		}
		else if( wParam == IDC_DELETE ){
			OnCustomizeDelete( hDlg );
		}
		else if( wParam == IDC_UP ){
			OnCustomizeUpDown( hDlg, -1 );
		}
		else if( wParam == IDC_DOWN ){
			OnCustomizeUpDown( hDlg, 1 );
		}
		else if( wParam == IDC_RESET ){
			TCHAR sz[260], szAppName[80];
			LoadString( EEGetLocaleInstanceHandle(), IDS_SURE_RESET_QUERIES, sz, _countof( sz ) );
			LoadString( EEGetLocaleInstanceHandle(), IDS_WORDCOUNT_MENU_TEXT, szAppName, sizeof( szAppName ) / sizeof( TCHAR ) );
			if( MessageBox( hDlg, sz, szAppName, MB_YESNO | MB_ICONEXCLAMATION ) == IDYES ){
				ResetArray();
				CustomizeRefreshList( hDlg, 0 );
				ResetList( m_hwndList );
				EraseEntry( szQueryArrayEntry );
				m_bRefreshOnClose = true;

//				m_bArrayModified = true;
			}
		}
	}

	BOOL OnCustomizeNotify( HWND hDlg, int idCtrl, LPNMHDR pnmh )
	{
		BOOL bResult = FALSE;
		if( idCtrl == IDC_LIST ){
			switch( pnmh->code ){
			case LVN_GETDISPINFO:
				{
					LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pnmh;
					if( pDispInfo->item.iItem < (int)m_Array.size() ) {
						_ASSERT( pDispInfo->item.iItem >= 0 && pDispInfo->item.iItem < (int)m_Array.size() );
						CQuery& query = m_Array[pDispInfo->item.iItem];
						if( pDispInfo->item.mask & LVIF_TEXT ){
							StringCopy( pDispInfo->item.pszText, pDispInfo->item.cchTextMax, query.m_sTitle.c_str() );
						}
					}
				}
				break;
			case NM_DBLCLK:
				{
					OnCustomizeProp( hDlg );
				}
				break;
			}
		}
		return bResult;
	}

	void CustomizeShowHide( HWND hDlg )
	{
		BOOL bRegExp = IsDlgButtonChecked( hDlg, IDC_CHECK_REGEXP );
		EnableWindow( GetDlgItem( hDlg, IDC_EDIT_REGEXP ), bRegExp );
		EnableWindow( GetDlgItem( hDlg, IDC_BROWSE_REG_EXP ), bRegExp );
	}

	void OnCustPropInitDialog( HWND hDlg )
	{
		CenterWindow( hDlg );

		SendDlgItemMessage( hDlg, IDC_TITLE, EM_LIMITTEXT, MAX_TITLE - 1, 0 );
		SendDlgItemMessage( hDlg, IDC_EDIT_REGEXP, EM_LIMITTEXT, MAX_TITLE - 1, 0 );
		SetDlgItemText( hDlg, IDC_TITLE, m_pProp->m_sTitle.c_str() );
		SetDlgItemText( hDlg, IDC_EDIT_REGEXP, m_pProp->m_sExp.c_str() );

		int nType = m_pProp->m_nType & TYPE_MASK;
		VERIFY( CheckRadioButton( hDlg, IDC_RADIO_CHARS, IDC_RADIO_PAGES, IDC_RADIO_CHARS + nType ) );
		VERIFY( CheckDlgButton( hDlg, IDC_CHECK_REGEXP, (m_pProp->m_nType & TYPE_REGEXP) ? TRUE : FALSE ) );

		CustomizeShowHide( hDlg );

	}

	void OnCustPropCommand( HWND hDlg, WPARAM wParam )
	{
		if( wParam == IDOK ){
			TCHAR sz[MAX_TITLE];
			GetDlgItemText( hDlg, IDC_TITLE, sz, _countof( sz ) );
			m_pProp->m_sTitle = sz;

			GetDlgItemText( hDlg, IDC_EDIT_REGEXP, sz, _countof( sz ) );
			m_pProp->m_sExp = sz;

			m_pProp->m_nType = TYPE_CHARS;
			if( IsDlgButtonChecked( hDlg, IDC_RADIO_WIDTHS ) ) {
				m_pProp->m_nType = TYPE_WIDTHS;
			}
			else if( IsDlgButtonChecked( hDlg, IDC_RADIO_WORDS ) ) {
				m_pProp->m_nType = TYPE_WORDS;
			}
			else if( IsDlgButtonChecked( hDlg, IDC_RADIO_LINES ) ) {
				m_pProp->m_nType = TYPE_LINES;
			}
			else if( IsDlgButtonChecked( hDlg, IDC_RADIO_VIEW_LINES ) ) {
				m_pProp->m_nType = TYPE_VIEW_LINES;
			}
			else if( IsDlgButtonChecked( hDlg, IDC_RADIO_PAGES ) ) {
				m_pProp->m_nType = TYPE_PAGES;
			}
			if( IsDlgButtonChecked( hDlg, IDC_CHECK_REGEXP ) ){
				m_pProp->m_nType |= TYPE_REGEXP;
				MATCH_REGEX_INFO_EX MatchRegexInfo;
				ZeroMemory( &MatchRegexInfo, sizeof( MatchRegexInfo ) );
				MatchRegexInfo.cbSize = sizeof( MatchRegexInfo );
				MatchRegexInfo.nFlags = FLAG_FIND_CASE;
				MatchRegexInfo.pszRegex = m_pProp->m_sExp.c_str();
				MatchRegexInfo.pszText = L"";
				if( Editor_MatchRegex( m_hWnd, &MatchRegexInfo ) == -1 ){
					PostMessage( hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem( hDlg, IDC_EDIT_REGEXP ), TRUE );
					return;
				}
			}
			EndDialog( hDlg, IDOK );
		}
		else if( wParam == IDCANCEL ){
			EndDialog( hDlg, IDCANCEL );
		}
		else if( wParam == IDC_CHECK_REGEXP ){
			CustomizeShowHide( hDlg );
		}
		//else if( wParam >= IDC_RADIO_CHARS && wParam <= IDC_RADIO_LINES ){
		//}
		//else if( wParam == MAKEWPARAM( IDC_EDIT_REGEXP, EN_CHANGE ) || wParam == MAKEWPARAM( IDC_TITLE, EN_CHANGE ) ){
		//	if( GetFocus() == GetDlgItem( hDlg, LOWORD( wParam ) ) ) {
		//	}
		//}
		else if( wParam == IDC_BROWSE_REG_EXP ){
			HWND hwndButton = (HWND)GetDlgItem( hDlg, (int)wParam );
			RECT rect;
			GetWindowRect( hwndButton, &rect );
			BOOL bRightAlign = GetSystemMetrics( SM_MENUDROPALIGNMENT );
			HMENU hMenu = LoadMenu( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE( IDR_WORDCOUNT_ARG_POPUP ) );
			HMENU hSubMenu = GetSubMenu( hMenu, 0 );
			UINT uID = TrackPopupMenu( hSubMenu, (bRightAlign ? TPM_RIGHTALIGN : TPM_LEFTALIGN) | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, bRightAlign ? rect.right : rect.left, rect.bottom, 0, hDlg, NULL );
			if( uID != 0 ){
				TCHAR sz[80];
				if( GetMenuString( hSubMenu, uID, sz, _countof( sz ), MF_BYCOMMAND ) ){
					LPTSTR p = _tcschr( sz, '\t' );
					if( p )  *p = 0;
					SendDlgItemMessage( hDlg, IDC_EDIT_REGEXP, EM_REPLACESEL, TRUE, (LPARAM)sz );
				}
			}
			DestroyMenu( hMenu );
		}
	}

	BOOL OnPropInitDialog( HWND hwnd )
	{
		CenterWindow( hwnd );
		TCHAR sz[256];
		for( int i = 0; i < 4; i++ ){
			LoadString( EEGetLocaleInstanceHandle(), IDS_POS_LEFT + i, sz, _countof( sz ) );
			SendDlgItemMessage( hwnd, IDC_COMBO_POS, CB_ADDSTRING, 0, (LPARAM)sz );
		}
		SendDlgItemMessage( hwnd, IDC_COMBO_POS, CB_SETCURSEL, m_iPos, 0 );
		LoadString( EEGetLocaleInstanceHandle(), IDS_WORDCOUNT_MENU_TEXT, sz, _countof( sz ) );
		SetWindowText( hwnd, sz );
		m_iOldPos = m_iPos;
		return TRUE;
	}
	
	void OnPropDlgCommand( HWND hwnd, WPARAM wParam )
	{
		switch( wParam ){
		case IDOK:
			{
				m_iPos = (int)SendDlgItemMessage( hwnd, IDC_COMBO_POS, CB_GETCURSEL, 0, 0 );
				WriteProfileInt( _T("CustomBarPos"), m_iPos );

				EndDialog( hwnd, IDOK );
				if( m_iPos != m_iOldPos ){
					if( m_hDlg ){
						OnCommand( NULL );
						OnCommand( NULL );
					}
				}
			}
			break;

		case IDCANCEL:
			EndDialog( hwnd, IDCANCEL );
			break;
		}
		return;
	}

	void OnContextMenu( LPARAM lParam )
	{
		HMENU hMainMenu = LoadMenu( EEGetLocaleInstanceHandle(), MAKEINTRESOURCE(IDR_WORDCOUNT_CONTEXT_MENU) );
		if( hMainMenu == NULL )  return;
		HMENU hMenu = GetSubMenu( hMainMenu, 0 );
		if( hMenu == NULL )  return;
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam);
		if( xPos == -1 && yPos == -1 ){
			POINT ptPos;
			ptPos.x = 16;
			ptPos.y = 16;
			ClientToScreen( m_hDlg, &ptPos );
			xPos = ptPos.x;
			yPos = ptPos.y;
		}
		if( ListView_GetSelectedCount( m_hwndList ) == 0 ) {
			EnableMenuItem( hMenu, ID_EDIT_COPY, MF_GRAYED );
		}
		int nID = TrackPopupMenu( hMenu, (GetSystemMetrics( SM_MENUDROPALIGNMENT ) ? TPM_RIGHTALIGN : TPM_LEFTALIGN) | TPM_RIGHTBUTTON | TPM_RETURNCMD, xPos, yPos, 0, m_hwndList, NULL );
		DestroyMenu( hMainMenu );

		switch( nID ){
		case ID_REFRESH:
			OnRefresh();
			break;
		case ID_CUSTOMIZE:
			OnCustomize( m_hDlg );
			break;
		case ID_PROP:
			SetProperties( m_hDlg );
			break;
		case ID_EDIT_COPY:
			OnCopy();
			break;
		case ID_SELECT_ALL:
			OnSelectAll();
			break;
		}
	}

	void SetListColor()
	{
		if( m_hwndList ){
			//COLORREF clr = ListView_GetBkColor( m_hwndList );
			//clr = ListView_GetTextBkColor( m_hwndList );
			//clr = ListView_GetTextColor( m_hwndList );
			DWORD dwColor = (DWORD)Editor_Info( m_hWnd, EI_GET_BAR_BACK_COLOR, SMART_COLOR_NORMAL );
			ListView_SetBkColor( m_hwndList, dwColor );
			ListView_SetTextBkColor( m_hwndList, dwColor );
			dwColor = (DWORD)Editor_Info( m_hWnd, EI_GET_BAR_TEXT_COLOR, SMART_COLOR_NORMAL );
			ListView_SetTextColor( m_hwndList, dwColor );
			InvalidateRect( m_hwndList, NULL, TRUE );
		}
	}

};

INT_PTR CALLBACK CustomizeDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	LRESULT nResult = 0;
	switch( msg ){
	case WM_INITDIALOG:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			if( pFrame ){
				pFrame->OnCustomizeInitDialog( hwnd );
			}
		}
		break;

	case WM_COMMAND:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			pFrame->OnCustomizeCommand( hwnd, wParam );
		}
		break;

	case WM_NOTIFY:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			_ASSERTE( pFrame );
			nResult = pFrame->OnCustomizeNotify( hwnd, (int)wParam, (LPNMHDR)lParam );
		}
		break;
	}
	return (BOOL)nResult;
}


INT_PTR CALLBACK CustPropDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM /*lParam*/ )
{
	LRESULT nResult = 0;
	switch( msg ){
	case WM_INITDIALOG:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			if( pFrame ){
				pFrame->OnCustPropInitDialog( hwnd );
			}
		}
		break;

	case WM_COMMAND:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			pFrame->OnCustPropCommand( hwnd, wParam );
		}
		break;

	//case WM_NOTIFY:
	//	{
	//		CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
	//		_ASSERTE( pFrame );
	//		nResult = pFrame->OnCustPropNotify( hwnd, wParam, (LPNMHDR)lParam );
	//	}
	//	break;
	}
	return (BOOL)nResult;
}


INT_PTR CALLBACK MainProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	BOOL bResult = FALSE;
	switch( msg ){
	case WM_INITDIALOG:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			bResult = pFrame->OnInitDialog( hwnd );
		}
		break;
	case WM_COMMAND:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			pFrame->OnDlgCommand( hwnd, LOWORD( wParam ) );
		}
		break;
	case WM_SIZE:
		{
			RECT rcDlg, rcList, rcProgress, rcAutoRefresh, rcBtn, rcBtnC;
			if( GetClientRect( hwnd, &rcDlg ) ){
				int cxDlg = rcDlg.right;
				int cyDlg = rcDlg.bottom;
				if( cxDlg == 0 || cyDlg == 0 )  break;

				HWND hwndList = GetDlgItem( hwnd, IDC_LIST );
				GetWindowRect( hwndList, &rcList );
				ScreenToClient( hwnd, (POINT*)&rcList.left );
				ScreenToClient( hwnd, (POINT*)&rcList.right );
				int cxEdge = rcList.left;

				HWND hwndProgress = GetDlgItem( hwnd, IDC_PROGRESS );
				GetWindowRect( hwndProgress, &rcProgress );
				ScreenToClient( hwnd, (POINT*)&rcProgress.left );
				ScreenToClient( hwnd, (POINT*)&rcProgress.right );

				HWND hwndAutoRefresh = GetDlgItem( hwnd, IDC_AUTO_REFRESH );
				GetWindowRect( hwndAutoRefresh, &rcAutoRefresh );
				ScreenToClient( hwnd, (POINT*)&rcAutoRefresh.left );
				ScreenToClient( hwnd, (POINT*)&rcAutoRefresh.right );

				HWND hwndBtn = GetDlgItem( hwnd, IDC_REFRESH );
				GetWindowRect( hwndBtn, &rcBtn );
				ScreenToClient( hwnd, (POINT*)&rcBtn.left );
				ScreenToClient( hwnd, (POINT*)&rcBtn.right );

				HWND hwndBtnC = GetDlgItem( hwnd, IDC_CUSTOMIZE );
				GetWindowRect( hwndBtnC, &rcBtnC );
				ScreenToClient( hwnd, (POINT*)&rcBtnC.left );
				ScreenToClient( hwnd, (POINT*)&rcBtnC.right );

				int cyListBottom = rcBtn.bottom - rcList.bottom;
				int cyProgressBottom = rcBtn.bottom - rcProgress.top;
				int cyAutoRefreshBottom = rcBtn.bottom - rcAutoRefresh.top;

				SetWindowPos( hwndList, NULL, 0, 0, cxDlg - cxEdge * 2, cyDlg - cxEdge - cyListBottom, SWP_NOMOVE | SWP_NOZORDER );
				SetWindowPos( hwndProgress, NULL, cxEdge, cyDlg - cyProgressBottom - cxEdge, cxDlg - cxEdge * 2, rcProgress.bottom - rcProgress.top, SWP_NOZORDER );
				SetWindowPos( hwndAutoRefresh, NULL, cxEdge, cyDlg - cyAutoRefreshBottom - cxEdge, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				SetWindowPos( hwndBtn, NULL, cxEdge, cyDlg - (rcBtn.bottom - rcBtn.top) - cxEdge, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				SetWindowPos( hwndBtnC, NULL, rcBtnC.left, cyDlg - (rcBtn.bottom - rcBtn.top) - cxEdge, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
			}
		}
		break;
	case WM_CONTEXTMENU:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			if( pFrame ){
				pFrame->OnContextMenu( lParam );
			}
		}
		break;
	case WM_DPICHANGED_AFTERPARENT:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
			if( pFrame ){
				pFrame->OnDpiChangedAfter();
			}
		}
		break;
//	case WM_NOTIFY:
//		{
//			NMHDR* pnmh = (NMHDR*)lParam;
//			switch( pnmh->code ){
////			case NM_DBLCLK:
//			case NM_CLICK:
//			case LVN_ITEMACTIVATE:
//				{
//					NMITEMACTIVATE* pNMItemActivate = (NMITEMACTIVATE*)pnmh;
//					if( pNMItemActivate->iItem >= 0 ){
//						CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrame( hwnd ));
//						pFrame->OnJump( pNMItemActivate->iItem );
//					}
//				}
//				break;
//			}
//		}
//		break;
	}
	return bResult;

}


INT_PTR CALLBACK PropDlg( HWND hwnd, UINT msg, WPARAM wParam, LPARAM /*lParam*/ )
{
	BOOL bResult = FALSE;
	switch( msg ){
	case WM_INITDIALOG:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrameFromDlg( hwnd ));
			_ASSERTE( pFrame );
			bResult = pFrame->OnPropInitDialog( hwnd );
		}
		break;
	case WM_COMMAND:
		{
			CMyFrame* pFrame = static_cast<CMyFrame*>(GetFrameFromDlg( hwnd ));
			_ASSERTE( pFrame );
			pFrame->OnPropDlgCommand( hwnd, wParam );
		}
		break;
	}
	return bResult;
}


int ExceptionHandler( HWND hwnd )
{
	int nResult = MessageBox( hwnd, _T("WordCount Plug-in WorkThread error! Click OK and restart EmEditor, or click Cancel to debug"), _T("WordCount"), MB_ICONSTOP | MB_OKCANCEL );
	if( nResult == IDOK ){
		return EXCEPTION_EXECUTE_HANDLER;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


UINT WINAPI WorkThread(LPVOID lpData)
{
	CMyFrame* pFrame = static_cast<CMyFrame*>(lpData);
	_ASSERTE( pFrame );
	HANDLE hQueEvent = pFrame->m_hQueEvent;
	_ASSERTE( hQueEvent );
//	HANDLE hMutex = pFrame->m_hMutex;

	while( !pFrame->m_bAbortThread ){
		if( WaitForSingleObject( hQueEvent, INFINITE ) != WAIT_OBJECT_0 ) {
			break;  // error
		}
		//if( WaitForSingleObject( hMutex, INFINITE ) != WAIT_OBJECT_0 ){
		//	break;  // error
		//}
		if( pFrame->m_bAbortThread ){
//			VERIFY( ReleaseMutex( hMutex ) );
			break;
		}
		ResetEvent( hQueEvent );

		__try {
			pFrame->RefreshList();
		}
		__except( ExceptionHandler( pFrame->m_hWnd ) ){
			_resetstkoflw();
			pFrame->m_bAbortThread = true;
		}

//		VERIFY( ReleaseMutex( hMutex ) );
//		TRACE( _T("WorkThread -- Mutex released.\n") );
	}

	pFrame->m_bAbortThread = false;
	return 0;
}


// the following line is needed after CMyFrame definition
_ETL_IMPLEMENT

