
// SDCardBurnDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SDCardBurn.h"
#include "SDCardBurnDlg.h"
#include "afxdialogex.h"
#include <winioctl.h>
#include <Dbt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//WriteSDCardThread *pThWrite;



CFile m_CFSrcFile;
HANDLE m_HdSDCard;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

														// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSDCardBurnDlg �Ի���



CSDCardBurnDlg::CSDCardBurnDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SDCARDBURN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSDCardBurnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_CBDisk);
	DDX_Control(pDX, IDC_EDIT_PATH, m_CEFilePath);
	DDX_Control(pDX, IDC_BUTTON_SELFILE, m_BuSelFile);
	DDX_Control(pDX, IDC_BUTTON_BURN, m_BuBurn);
	DDX_Control(pDX, IDC_PROGRESS, m_CProgress);
	DDX_Control(pDX, IDC_STATIC_STATE, m_CSState);
}

BEGIN_MESSAGE_MAP(CSDCardBurnDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SELFILE, &CSDCardBurnDlg::OnBnClickedButtonSelfile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_BURN, &CSDCardBurnDlg::OnBnClickedButtonBurn)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSDCardBurnDlg ��Ϣ�������

BOOL CSDCardBurnDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

									// TODO: �ڴ���Ӷ���ĳ�ʼ������

									//���ڱ���
	this->SetWindowTextW(L"SD����д����V2.1");

	WCHAR	curDire[MAX_PATH];
	CString strPath;
	GetCurrentDirectory(MAX_PATH, curDire);
	strPath.Format(L"%s", curDire);
	strPath = strPath.Left(strPath.ReverseFind(L'\\'));
	strPath += L"\\ACC3.bin";
	//strPath.Format(L"%s\\ACC3.bin", curDire);
	m_CEFilePath.SetWindowTextW(strPath);
	//pThWrite = (WriteSDCardThread *)AfxBeginThread(RUNTIME_CLASS(WriteSDCardThread), 0, 1024 * 1024 * 1, CREATE_SUSPENDED);
	ScanAllDisk();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSDCardBurnDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSDCardBurnDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSDCardBurnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CSDCardBurnDlg::MainRun(LPVOID lParam)
{


	return 0;
}

#define RDBUFF_SIZE	(1024*1024*2)
CHAR	rdBuffer[RDBUFF_SIZE], verBuffer[RDBUFF_SIZE];

UINT CSDCardBurnDlg::WriteData(LPVOID lParam)
{
	BOOL	bRtn;
	INT		verDiff;
	DWORD	outSize;
	UINT	rdLen;
	ULONGLONG	fsSize, dealSize;
	LONG	sdPoint;
	UINT	progValue;
	CString staticStr;

	CSDCardBurnDlg *myDlg = (CSDCardBurnDlg *)lParam;
	CProgressCtrl *progress = &myDlg->m_CProgress;
	CStatic *myStatic = &myDlg->m_CSState;

	CButton *myBuFilePath = &myDlg->m_BuSelFile;
	CButton *myBuBurn = &myDlg->m_BuBurn;
	CEdit *myEditPath = &myDlg->m_CEFilePath;

	myEditPath->EnableWindow(false);
	myBuBurn->EnableWindow(false);
	myBuFilePath->EnableWindow(false);

	//ɾ������������Ϣ(����0��һ��ƫ����Ϣ�������������ᵼ��д��ĵ�ַ����Ҫ�Ĳ�һ��)
	bRtn = DeviceIoControl(m_HdSDCard, IOCTL_DISK_DELETE_DRIVE_LAYOUT, NULL, 0, NULL, 0, &outSize, NULL);
	if (bRtn == FALSE)
	{
		myDlg->ViewLastError(L"ɾ��������Ϣʧ��:");
		goto __LAB_ERROR;
	}
	//�������0
	memset(rdBuffer, 0, 512);
	SetFilePointer(m_HdSDCard, 0, 0, FILE_BEGIN);
	bRtn = WriteFile(m_HdSDCard, rdBuffer, 512, &outSize, NULL);
	if (bRtn == FALSE)
	{
		myDlg->ViewLastError(L"�������0ʧ��:");
		goto __LAB_ERROR;
	}
	//����SD����ƫ�Ƶ�
	sdPoint = 1024 * 1024 * 1;
	//Դ�ļ�����
	fsSize = m_CFSrcFile.GetLength();
	dealSize = 0;
	//��ʼд�ļ�
	while (dealSize < fsSize)
	{
		rdLen = m_CFSrcFile.Read(rdBuffer, RDBUFF_SIZE);
		if (rdLen)
		{
			//д��SD��
			SetFilePointer(m_HdSDCard, sdPoint, NULL, FILE_BEGIN);
			bRtn = WriteFile(m_HdSDCard, rdBuffer, rdLen, &outSize, NULL);
			if (bRtn == FALSE)
			{
				myDlg->ViewLastError(L"д��SD��ʧ��:");
				goto __LAB_ERROR;
			}
			//��SD������
			SetFilePointer(m_HdSDCard, sdPoint, NULL, FILE_BEGIN);
			bRtn = ReadFile(m_HdSDCard, verBuffer, rdLen, &outSize, NULL);
			if (bRtn == FALSE)
			{
				myDlg->ViewLastError(L"��SD����������ʧ��:");
				goto __LAB_ERROR;
			}
			//У��
			verDiff = memcmp(rdBuffer, verBuffer, rdLen);
			if (verDiff != 0)
			{
				AfxMessageBox(L"У�����ݳ���", MB_OK | MB_ICONERROR);
				goto __LAB_ERROR;
			}
			sdPoint += rdLen;
			dealSize += rdLen;

			progValue = (INT)(((float)dealSize / (float)fsSize) * 100);
			progress->SetPos(progValue);
			staticStr.Format(L"%u%%", progValue);
			myStatic->SetWindowTextW(staticStr);
		}
		//Sleep(10);
	}

	progress->SetPos(100);
	AfxMessageBox(L"д���ݳɹ�", MB_OK | MB_ICONMASK);

__LAB_ERROR:
	myEditPath->EnableWindow(true);
	myBuBurn->EnableWindow(true);
	myBuFilePath->EnableWindow(true);
	CloseHandle(m_HdSDCard);
	m_CFSrcFile.Close();
	AfxEndThread(0);
	return 0;
}



void CSDCardBurnDlg::OnBnClickedButtonSelfile()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString filePath;
	BOOL bRes;
	WCHAR	curDire[MAX_PATH];
	CFileDialog dlgFile(true, NULL, L"", OFN_HIDEREADONLY, L"bin�ļ�(*.bin)|*.bin|", NULL);
	OPENFILENAME& ofn = dlgFile.GetOFN();
	GetCurrentDirectory(MAX_PATH, curDire);

	ofn.lpstrInitialDir = curDire;
	dlgFile.ApplyOFNToShellDialog();
	if (dlgFile.DoModal() == IDOK)
	{
		filePath = dlgFile.GetPathName();
		m_CEFilePath.SetWindowTextW(filePath);
	}
	else
	{
		//MessageBox(L"ѡ���ļ�����", L"����", MB_OK | MB_ICONERROR);
	}
}


void CSDCardBurnDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnClose();
}


void CSDCardBurnDlg::OnBnClickedButtonBurn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	WCHAR textBuff[1024];
	CString str;
	BOOL bRtn;

	//��ȡԴ�ļ�·��
	m_CEFilePath.GetWindowTextW(textBuff, m_CEFilePath.GetWindowTextLengthW() + 1);
	str = textBuff;
	if (str.IsEmpty())
	{
		MessageBox(L"·������Ϊ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	//��Դ�ļ�
	bRtn = m_CFSrcFile.Open(str, CFile::modeRead | CFile::typeBinary | CFile::shareDenyRead);
	if (!bRtn)
	{
		MessageBox(L"��Դ�ļ�ʧ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}

	//�������̾��
	//m_CBDisk.GetWindowTextW(str);
	if (m_CBDisk.GetCount() == 0)
	{
		m_CFSrcFile.Close();
		MessageBox(L"��ѡ�����", L"����", MB_OK | MB_ICONWARNING);
		return;
	}
	m_CBDisk.GetLBText(m_CBDisk.GetCurSel(), str);
	if (str.IsEmpty())
	{
		MessageBox(L"��ѡ�����", L"����", MB_OK | MB_ICONWARNING);
		m_CFSrcFile.Close();
		return;
	}
	str = L"\\\\.\\" + str;
	m_HdSDCard = CreateFile(str, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (m_HdSDCard == INVALID_HANDLE_VALUE)
	{
		m_CFSrcFile.Close();
		AfxMessageBox(L"��SD������", MB_OK | MB_ICONERROR);
		return;
	}

	//m_TimerID = SetTimer(0, 10, 0);
	m_CSState.SetWindowTextW(L"0%");
	m_CProgress.SetPos(0);
	m_TFunWriteData = AfxBeginThread((AFX_THREADPROC)WriteData, this, THREAD_PRIORITY_NORMAL, 1024 * 1024 * 1, CREATE_SUSPENDED, NULL);
	m_TFunWriteData->ResumeThread();
}


void CSDCardBurnDlg::ScanAllDisk(void)
{
	TCHAR szBuf[100];
	DWORD len, i;
	UINT type;
	CString str;
	memset(szBuf, 0, 100);

	len = GetLogicalDriveStrings(sizeof(szBuf) / sizeof(TCHAR), szBuf);
	m_CBDisk.ResetContent();

	for (i = 0; i<len; i++)
	{
		str = &szBuf[i];
		type = GetDriveType(str);
		if (type == DRIVE_REMOVABLE)
		{
			str.Delete(2, 1);
			m_CBDisk.AddString(str);
		}
	}
	i = m_CBDisk.GetCount();
	if (i > 0)
	{
		m_CBDisk.SetCurSel(i - 1);
	}
	else
	{
		m_CBDisk.SetCurSel(0);
	}
}

void CSDCardBurnDlg::ViewLastError(LPTSTR lpMsg)
{

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();//��ȡ�������

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	CString strErr;
	strErr.Format(L"%s%s", lpMsg, lpMsgBuf);
	MessageBox(strErr, L"����", MB_OK | MB_ICONERROR);
	LocalFree(lpMsgBuf);
}




LRESULT CSDCardBurnDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//WM_DEVICECHANGE��ϵͳӲ���ı䷢����ϵͳ��Ϣ
	case WM_DEVICECHANGE:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		switch (wParam)
		{
		case WM_DEVICECHANGE:
			break;
		case DBT_DEVICEARRIVAL://DBT_DEVICEARRIVAL���豸�����������ҿ���ʹ��
		{
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//�߼���
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				switch (lpdbv->dbcv_flags)
				{
				case 0:                //U��
				{
					ScanAllDisk();
				}
				break;
				case DBTF_MEDIA:    //����
					break;
				}
			}
		}
		break;
		case DBT_DEVICEREMOVECOMPLETE://DBT_DEVICEREMOVECOMPLETE,�豸ж�ػ��߰γ�
		{
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//�߼���
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				switch (lpdbv->dbcv_flags)
				{
				case 0:                //U��
				{
					ScanAllDisk();
				}
				break;
				case DBTF_MEDIA:    //����

					break;
				}
			}
		}
		break;
		}
	}
	break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}


void CSDCardBurnDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ


	CDialogEx::OnTimer(nIDEvent);
}
