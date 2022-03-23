#include <SFML/Graphics.hpp>
#include <ctime>

// TODO:
// - statically link
// - add icon
// - make it tray only
// - parameters
// - improve border aliasing

// DONE:
// - remove console
// - reduce resources usage
// - make it draggable

#define pow2(x) ((x)*(x))

#if defined (SFML_SYSTEM_WINDOWS)
#include <windows.h>

bool setShapeCircle(HWND hWnd, const sf::Vector2i& size)
{
    HRGN hRegion = CreateRectRgn(0, 0, size.x, size.y);

    // Determine the visible region
    int d2 = pow2(size.x / 2);
    for (int y = 0; y < size.y; y++)
    {
        for (int x = 0; x < size.x; x++)
        {
            int d = pow2(x - size.x / 2) + pow2(y - size.y / 2);
			if (d > d2)
            {
                HRGN hRegionPixel = CreateRectRgn(x, y, x + 1, y + 1);
                CombineRgn(hRegion, hRegion, hRegionPixel, RGN_XOR);
                DeleteObject(hRegionPixel);
            }
        }
    }

    SetWindowRgn(hWnd, hRegion, true);
    DeleteObject(hRegion);
    return true;
}

bool setTransparency(HWND hWnd, unsigned char alpha)
{
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
    return true;
}

#elif defined (SFML_SYSTEM_LINUX)
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

bool setShapeCircle(Window wnd, const sf::Vector2i& size)
{
    Display* display = XOpenDisplay(NULL);

    // Try to set the window shape
    int event_base;
    int error_base;
    if (XShapeQueryExtension(display, &event_base, &error_base))
    {
        Pixmap pixmap = XCreatePixmap(display, wnd, size.x, size.y, 1);
        GC gc = XCreateGC(display, pixmap, 0, NULL);

        XSetForeground(display, gc, 1);
        XFillRectangle(display, pixmap, gc, 0, 0, size.x, size.y);
        XSetForeground(display, gc, 0);

        int d2 = pow2(size.x / 2);
        for (unsigned int y = 0; y < size.y; y++)
        {
            for (unsigned int x = 0; x < size.x; x++)
            {
                int d = pow2(x - size.x / 2) + pow2(y - size.y / 2);
                if (d > d2)
                {
                    XFillRectangle(display, pixmap, gc, x, y, 1, 1);
                }
            }
        }

        XShapeCombineMask(display, wnd, ShapeBounding, 0, 0, pixmap, ShapeSet);
        XFreeGC(display, gc);
        XFreePixmap(display, pixmap);
        XFlush(display);
        XCloseDisplay(display);
        return true;
    }

    XCloseDisplay(display);
}

bool setTransparency(Window wnd, unsigned char alpha)
{
    Display* display = XOpenDisplay(NULL);
    unsigned long opacity = (0xffffffff / 0xff) * alpha;
    Atom property = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", false);
    if (property != None)
    {
        XChangeProperty(display, wnd, property, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&opacity, 1);
        XFlush(display);
        XCloseDisplay(display);
        return true;
    }
    else
    {
        XCloseDisplay(display);
        return false;
    }
}

#undef None // None conflicts with SFML
#elif defined (SFML_SYSTEM_MACOS)
bool setShapeCircle(sf::WindowHandle handle, const sf::Vector2i& size);
bool setTransparency(sf::WindowHandle handle, unsigned char alpha);
#else
bool setShapeCircle(sf::WindowHandle handle, const sf::Vector2i& size)
{
    return false;
}

bool setTransparency(sf::WindowHandle handle, unsigned char alpha)
{
    return false;
}
#endif

int wsize = 200;
bool stop2go = false;
int posx = 0;
int posy = 0;

// parameters
#define scaling ((float)wsize/360.f)

#define clock_r  (180 *scaling)
#define clock_b  ( 12 *scaling)

#define ticks_r  (160 *scaling)
#define ticks_l1 ( 40 *scaling)
#define ticks_w1 ( 12 *scaling)
#define ticks_l2 ( 12 *scaling)
#define ticks_w2 (  5 *scaling)

#define hour_l1  ( 94 *scaling)
#define hour_l2  ( 39 *scaling)
#define hour_w1  ( 15 *scaling)
#define hour_w2  ( 21 *scaling)

#define min_l1   (152 *scaling)
#define min_l2   ( 40 *scaling)
#define min_w1   ( 12 *scaling)
#define min_w2   ( 18 *scaling)

#define sec_l1   (122 *scaling)
#define sec_l2   ( 50 *scaling)
#define sec_w    (  5 *scaling)
#define sec_r    ( 13 *scaling)

#define colhex_white 0xfdfdfdff
#define colhex_black 0x221e20ff
#define colhex_red   0xec2324ff

// TODO: create rendertexture for static elements to reduce the number of draw calls
void DrawMondaineClock(sf::RenderWindow& window, int h, int m, int s)
{
	sf::Color white(colhex_white);
	sf::Color black(colhex_black);
	sf::Color red(colhex_red);

	sf::Vector2f center(clock_r, clock_r);

	// draw background + border
	{
		sf::CircleShape cs(clock_r - clock_b, 64);
		cs.setFillColor(white);
		cs.setOutlineThickness(clock_b);
		cs.setOutlineColor(black);
		cs.setPosition(sf::Vector2f(clock_b, clock_b));
		window.draw(cs);
	}

	// draw ticks
	{
		sf::RectangleShape rs;
		rs.setFillColor(black);
		for (int i = 0; i < 60; i++)
		{
			sf::Vector2f size;
			if (i % 5 == 0) // large tick
			{
				size = sf::Vector2f(ticks_w1, ticks_l1);
			}
			else // small tick
			{
				size = sf::Vector2f(ticks_w2, ticks_l2);
			}
			float angle = (float)i * 6.f; // / 60.f * 360.f;
			rs.setOrigin(size.x / 2.f, ticks_r);
			rs.setPosition(center);
			rs.setRotation(angle);
			rs.setSize(size);
			window.draw(rs);
		}
	}

	// draw hours
	{
		sf::RectangleShape rs;
		rs.setFillColor(black);
		float angle = (float)(h % 12) * 30.f + (float)(m % 60) * 0.5f;
		sf::Vector2f size = sf::Vector2f((hour_w1 + hour_w2) / 2.f, hour_l1 + hour_l2);
		rs.setOrigin(size.x / 2.f, hour_l1);
		rs.setPosition(center);
		rs.setRotation(angle);
		rs.setSize(size);
		window.draw(rs);
	}

	// draw minutes
	{
		sf::RectangleShape rs;
		rs.setFillColor(black);
		float angle = (float)(m % 60) * 6.f;
		sf::Vector2f size = sf::Vector2f((min_w1 + min_w2) / 2.f, min_l1 + min_l2);
		rs.setOrigin(size.x / 2.f, min_l1);
		rs.setPosition(center);
		rs.setRotation(angle);
		rs.setSize(size);
		window.draw(rs);
	}

	// draw seconds
	{
		sf::RectangleShape rs;
		rs.setFillColor(red);
		float angle = (float)(s % 60) * 6.f;
		sf::Vector2f size = sf::Vector2f(sec_w, sec_l1 + sec_l2);
		rs.setOrigin(size.x / 2.f, sec_l1);
		rs.setPosition(center);
		rs.setRotation(angle);
		rs.setSize(size);
		window.draw(rs);

		sf::CircleShape cs(sec_r, 64);
		cs.setFillColor(red);
		cs.setOrigin(sec_r, sec_l1);
		cs.setPosition(center);
		cs.setRotation(angle);
		window.draw(cs);
	}
}

// parameters -size=200 -posx=X -posy=X -stop2go

void ReadParams(int argc, char** argv)
{

}

int main();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    main();
}

int main()
{
    //FreeConsole();
    //ReadParams(argc, argv);
    sf::Vector2i windowSize(wsize, wsize);
    sf::Vector2i windowPosition(-wsize-10, 2160-wsize-10-60);
        //(sf::VideoMode::getDesktopMode().width  - windowSize.x) / 2,
        //(sf::VideoMode::getDesktopMode().height - windowSize.y) / 2);


    // Create the window and center it on the screen
    sf::ContextSettings contextSettings;
    contextSettings.antialiasingLevel = 10;
    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y, 32), "Mondaine Clock", sf::Style::None, contextSettings);
    window.setPosition(windowPosition);
    window.setFramerateLimit(1);

    // set icon
    //sf::Image icon;
    //icon.loadFromFile("icon.png"); // File/Image/Pixel
    //window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    //setShapeCircle(window.getSystemHandle(), windowSize);
    //setTransparency(window.getSystemHandle(), 255);

    bool isMouseDragging = false;
    int lastDownX = 0, lastDownY = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || (event.key.code == sf::Keyboard::Escape && event.type == sf::Event::KeyPressed))
            {
                window.close();
            }

            // drag window
            switch (event.type)
            {
            case sf::Event::MouseMoved:
                if (isMouseDragging)
                {
                    window.setPosition(window.getPosition() + sf::Vector2<int>(event.mouseMove.x - lastDownX, event.mouseMove.y - lastDownY));
                }
                break;
            case sf::Event::MouseButtonPressed:
                lastDownX = event.mouseButton.x;
                lastDownY = event.mouseButton.y;
                isMouseDragging = true;
                window.setFramerateLimit(30);
                break;
            case sf::Event::MouseButtonReleased:
                isMouseDragging = false;
                window.setFramerateLimit(1);
                break;
            }
        }

        window.clear(sf::Color(255, 0, 255, 255));
        //window.clear(sf::Color(colhex_black));

        // get the current time point
        const std::time_t now = std::time(nullptr);
        // convert it to (local) calendar time
        const std::tm calendar_time = *std::localtime(std::addressof(now));

        // TODO: stop2go feature

        int Hour = calendar_time.tm_hour;
        int Min  = calendar_time.tm_min;
        int Sec  = calendar_time.tm_sec;
        //int Msec = calendar_time.tm_
        if (stop2go)
        {
            float Secf = (float)(Sec - 30.f) * (60.f / 58.f) + 30.f;
            Secf = Secf < 0.f ? 0.f : Secf > 60.f ? 60.f : Secf;
            Sec = (int)round(Secf);
        }
            
        //Hour = 10; Min = 9; Sec = 37;
        DrawMondaineClock(window, Hour, Min, Sec);
        window.display();

        // TODO: sleep for some time
    }

    return 0;
}
