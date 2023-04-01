#define BUFSIZE 65535 
#define SHIFTED 0x8000 
//Apuntador a consola \\ Contexto Principal \\ Parametros de Palabra \\ Parametros de Linea
LONG APIENTRY MainWndProc(HWND hwndMain, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    HDC hdc;                   // Apuntador del setup de la Consola
    TEXTMETRIC tm;             // Estructura de parametros de texto
    static DWORD dwCharX;      // Ancho promedio de los caracteres 
    static DWORD dwCharY;      // Largo promedio de los caracteres
    static DWORD dwClientX;    // Ancho del area editable
    static DWORD dwClientY;    // Largo del area editable
    static DWORD dwLineLen;    // Limite de largo de linea
    static DWORD dwLines;      // Numero de lineas en area editable 
    static int nCaretPosX = 0; // Posicion X del Cursor
    static int nCaretPosY = 0; // Posicion Y del Cursor
    static int nCharWidth = 0; // Ancho del Caracter en Bits
    static int cch = 0;        // Numero de Caracteres en el buffer 
    static int nCurChar = 0;   // Indice del actual Caracter 
    static PTCHAR pchInputBuf; // Buffer de entrada 
    int i, j;                  // Contadores del ciclo
    int cCR = 0;               // Contador de retornos acumulados 
    int nCRIndex = 0;          // Indice del ultimo retorno acumulado 
    int nVirtKey;              // Codigo de la tecla Virtual
    TCHAR szBuf[128];          // Buffer de soporte 
    TCHAR ch;                  // Caracter Actual 
    PAINTSTRUCT ps;            // required by BeginPaint 
    RECT rc;                   // Rectangulo de salida para imprimir texto 
    SIZE sz;                   // Dimensiones de la cadena
    COLORREF crPrevText;       // Color del Texto
    COLORREF crPrevBk;         // Color de fondo
    size_t * pcch;
    HRESULT hResult; 
 
    switch (uMsg) { 
        case WM_CREATE: //Crea una nueva consola
            hdc = GetDC(hwndMain); 
            GetTextMetrics(hdc, &tm); //Se extrae la medicion de texto
            ReleaseDC(hwndMain, hdc); //Se asigna el HDC
 
            // Guardamos las dimensiones promedio 
            dwCharX = tm.tmAveCharWidth; 
            dwCharY = tm.tmHeight; 
 
            // Asignamos un Buffer para que almacene el input 
            pchInputBuf = (LPTSTR) GlobalAlloc(GPTR, BUFSIZE * sizeof(TCHAR)); 
            return 0; //Termina el proceso con un exito

        case WM_SIZE: //Modificar las dimensiones del area editable :: No confundir con la pantalla total
            dwClientX = LOWORD(lParam); //Usan los parametros de linea
            dwClientY = HIWORD(lParam); 
 
            // Calcula el ancho maximo de linea y
            // el numero maximo de lineas del area editable 
            
            dwLineLen = dwClientX - dwCharX; 
            dwLines = dwClientY / dwCharY; 
            break; //Proceso termina con un NULL
 
        case WM_SETFOCUS: //Establece el Cursor en la pantalla
            // Diseñado para la entrada de datos
 
            CreateCaret(hwndMain, (HBITMAP) 1, 0, dwCharY); //Crea el Cursor (apuntador a Consola,?,?,Largo de caracter) 
            SetCaretPos(nCaretPosX, nCaretPosY * dwCharY);  //Posicion del Cursor (X, Y)
            ShowCaret(hwndMain); //Parpadea el Cursor
            break; //Proceso termina en NULL
 
        case WM_KILLFOCUS: //Destruye el Cursor
            HideCaret(hwndMain); //Deja de Parpadear el Cursor
            DestroyCaret();  //Elimina el cursor
            break; 
 
        case WM_CHAR://Escucha del teclado
            // Se realiza un checkpoint si el input esta casi al desborde
            // Por lo que se admite la transmision
            // Esto no sera necesario con la string de c++
            if (cch > BUFSIZE-5){
                pchInputBuf[cch] = 0x00;//Caracter nulo
                SendMessage(hwndMain, WM_PAINT, 0, 0);
            } 
            switch (wParam) { //Este parametro debera hacerse con el estado general del teclado
                case 0x08:  // Retroceso
                case 0x0A:  // linefeed ???
                case 0x1B:  // ESCAPE
                    MessageBeep((UINT) -1); 
                    return 0; //
                case 0x09:  // Tabulador
                    // Por defecto Otorga 4 espacios 
                    for (i = 0; i < 4; i++) 
                        SendMessage(hwndMain, WM_CHAR, 0x20, 0); 
                    return 0; 
                case 0x0D:  // Enter
                    // Transporta a una linea nueva
                    pchInputBuf[cch++] = 0x0D; //Codigo del \n
                    nCaretPosX = 0; 
                    nCaretPosY += 1; 
                    break;

                default:    // Todo caso consideramos una tecla Desplegable
                    ch = (TCHAR) wParam; //Se extrae el caracter
                    HideCaret(hwndMain); 
                    hdc = GetDC(hwndMain); 
                    GetCharWidth32(hdc, (UINT) wParam, (UINT) wParam, &nCharWidth); //Obtiene el ancho de caracter
                    TextOut(hdc, nCaretPosX, nCaretPosY * dwCharY, &ch, 1); //Imprime el Caracter
                    ReleaseDC(hwndMain, hdc); 
                    // Guarda el caracter en el buffer
                    pchInputBuf[cch++] = ch; 
                    // Calcula la nueva posicion del cursor
                    // Si esta se desborda, iniciara en una nueva linea
                    nCaretPosX += nCharWidth; 
                    if ((DWORD) nCaretPosX > dwLineLen) { 
                        nCaretPosX = 0;
                        pchInputBuf[cch++] = 0x0D; 
                        ++nCaretPosY; 
                    } 
                    nCurChar = cch; 
                    ShowCaret(hwndMain); 
                    break; 
            } 
            SetCaretPos(nCaretPosX, nCaretPosY * dwCharY); //Posiciona el cursor en su nueva posicion
            break; 
 
        case WM_KEYDOWN: //Teclas de control presionadas (contexto de escritura)
            switch (wParam) { 
                case VK_LEFT:   // LEFT ARROW 
                    // Solo puede recorrerse hasta el inicio de la linea
                    if (nCaretPosX > 0) { 
                        HideCaret(hwndMain); 
                        // Recorre la posicion del caracter actual y calcula la nueva posicion
                        ch = pchInputBuf[--nCurChar]; 
                        hdc = GetDC(hwndMain); 
                        GetCharWidth32(hdc, ch, ch, &nCharWidth); 
                        ReleaseDC(hwndMain, hdc); 
                        nCaretPosX = max(nCaretPosX - nCharWidth, 
                            0); 
                        ShowCaret(hwndMain); 
                    } 
                    break; 
 
                case VK_RIGHT:  // RIGHT ARROW 
                    // Puede recorrerse a lo largo de todo lo escrito
                    if (nCurChar < cch) { 
                        HideCaret(hwndMain); 
                        //Si el caracter es un Enter \n entonces mueve a la siguiente linea
                        ch = pchInputBuf[nCurChar]; 
                        if (ch == 0x0D) { //Esto evita recolectar el \n
                            nCaretPosX = 0; 
                            nCaretPosY++; 
                        } else { //Remarca la seleccion de texto
                            hdc = GetDC(hwndMain); 
                            nVirtKey = GetKeyState(VK_SHIFT); 
                            if (nVirtKey & SHIFTED){ //Si se oprime -> Y SHIFT
                                crPrevText = SetTextColor(hdc, RGB(255, 255, 255)); 
                                crPrevBk = SetBkColor(hdc, RGB(0,0,0)); 
                                TextOut(hdc, nCaretPosX, nCaretPosY * dwCharY, &ch, 1); 
                                SetTextColor(hdc, crPrevText); 
                                SetBkColor(hdc, crPrevBk); //Color contraste de texto seleccionado
                            } 
                            //Recorre el cursor
                            GetCharWidth32(hdc, ch, ch, &nCharWidth); 
                            ReleaseDC(hwndMain, hdc); 
                            nCaretPosX = nCaretPosX + nCharWidth; 
                        } 
                        nCurChar++; 
                        ShowCaret(hwndMain); 
                        break; 
                    } 
                    break; 
 
                case VK_UP:     // UP ARROW 
                case VK_DOWN:   // DOWN ARROW 
                    MessageBeep((UINT) -1); 
                    return 0; //No hacen nada
                case VK_HOME:   // HOME
                    // Vuelve al origen 
                    nCaretPosX = nCaretPosY = 0; 
                    nCurChar = 0; 
                    break; 
                case VK_END:    // END  
                    //Lleva al ultimo extremo escrito
                    for (i=0; i < cch; i++) {
                        if (pchInputBuf[i] == 0x0D) { 
                            cCR++; 
                            nCRIndex = i + 1; 
                        } 
                    } 
                    nCaretPosY = cCR; 
 
                    //Recupera la cadena seleccionada
                    //Al parecer esta deberia ir en Control + C
 
                    for (i = nCRIndex, j = 0; i < cch; i++, j++) 
                        szBuf[j] = pchInputBuf[i]; 
                    szBuf[j] = TEXT('\0'); 
 
                    //Proceso de rescatar la frase de la cadena
 
                    hdc = GetDC(hwndMain);
                    hResult = StringCchLength(szBuf, 128, pcch);
                    if (FAILED(hResult)){
                    // TODO: write error handler
                    } 
                    GetTextExtentPoint32(hdc, szBuf, *pcch, &sz); 
                    nCaretPosX = sz.cx; 
                    ReleaseDC(hwndMain, hdc); 
                    nCurChar = cch; 
                    break; 
 
                default: 
                    break; 
            } 
            SetCaretPos(nCaretPosX, nCaretPosY * dwCharY); //Actualizar el cursor
            break; 
 
        case WM_PAINT: //Imprimir un segmento
            if (cch == 0)       // nothing in input buffer 
                break; 
 
            hdc = BeginPaint(hwndMain, &ps); 
            HideCaret(hwndMain); 
 
            // Set the clipping rectangle, and then draw the text 
            // into it. 
 
            SetRect(&rc, 0, 0, dwLineLen, dwClientY); 
            DrawText(hdc, pchInputBuf, -1, &rc, DT_LEFT); 
 
            ShowCaret(hwndMain); 
            EndPaint(hwndMain, &ps); 
            break; 
        
        case WM_DESTROY://Cerrar 
            PostQuitMessage(0); 
 
            // Free the input buffer. 
 
            GlobalFree((HGLOBAL) pchInputBuf); 
            UnregisterHotKey(hwndMain, 0xAAAA); 
            break; 
 
        default: 
            return DefWindowProc(hwndMain, uMsg, wParam, lParam); //Status Quo
    } 
    return NULL; 
}