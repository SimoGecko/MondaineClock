#include <SFML/Graphics.hpp>
//#include <chrono>
#include <ctime>

#if defined (SFML_SYSTEM_WINDOWS)
#include <windows.h>

bool setShape(HWND hWnd, const sf::Image& image)
{
    const sf::Uint8* pixelData = image.getPixelsPtr();
    HRGN hRegion = CreateRectRgn(0, 0, image.getSize().x, image.getSize().y);

    // Determine the visible region
    for (unsigned int y = 0; y < image.getSize().y; y++)
    {
        for (unsigned int x = 0; x < image.getSize().x; x++)
        {
            if (pixelData[y * image.getSize().x * 4 + x * 4 + 3] == 0)
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

#define pow2(x) ((x)*(x))

bool setShapeCircle(HWND hWnd, sf::Vector2i size)
{
    HRGN hRegion = CreateRectRgn(0, 0, size.x, size.y);

    // Determine the visible region
    int d2 = pow2(size.x / 2);// pow2(size.x / 2) + pow2(size.y / 2);
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

bool setShape(Window wnd, const sf::Image& image)
{
    const sf::Uint8* pixelData = image.getPixelsPtr();
    Display* display = XOpenDisplay(NULL);

    // Try to set the window shape
    int event_base;
    int error_base;
    if (XShapeQueryExtension(display, &event_base, &error_base))
    {
        Pixmap pixmap = XCreatePixmap(display, wnd, image.getSize().x, image.getSize().y, 1);
        GC gc = XCreateGC(display, pixmap, 0, NULL);

        XSetForeground(display, gc, 1);
        XFillRectangle(display, pixmap, gc, 0, 0, image.getSize().x, image.getSize().y);
        XSetForeground(display, gc, 0);

        for (unsigned int y = 0; y < image.getSize().y; y++)
        {
            for (unsigned int x = 0; x < image.getSize().x; x++)
            {
                if (pixelData[y * image.getSize().x * 4 + x * 4 + 3] == 0)
                    XFillRectangle(display, pixmap, gc, x, y, 1, 1);
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
bool setShape(sf::WindowHandle handle, const sf::Image& image);
bool setTransparency(sf::WindowHandle handle, unsigned char alpha);
#else
bool setShape(sf::WindowHandle handle, const sf::Image& image)
{
    return false;
}

bool setTransparency(sf::WindowHandle handle, unsigned char alpha)
{
    return false;
}
#endif


// parameters
#define clock_r 180
#define clock_b 10

#define ticks_r 160
#define ticks_l1 40
#define ticks_w1 12
#define ticks_l2 12
#define ticks_w2  5

#define hour_l1 94
#define hour_l2 39
#define hour_w1 15
#define hour_w2 21

#define min_l1 152
#define min_l2  40
#define min_w1  12
#define min_w2  18

#define sec_l1 122
#define sec_l2 50
#define sec_w   5
#define sec_r  13


void DrawMondaineClock(sf::RenderWindow& window, int h, int m, int s)
{
	sf::Color white(0xfdfdfdff);
	sf::Color black(0x221e20ff);
	sf::Color red(0xec2324ff);

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

int size = 360;

int main()
{
    // Change this to the wanted transparency
    const unsigned char opacity = 255;

    sf::Vector2i windowSize(size, size);

    // Load an image with transparent parts that will define the shape of the window
    //sf::Image backgroundImage;
    //backgroundImage.loadFromFile("image.png");


    // Create the window and center it on the screen
    sf::ContextSettings contextSettings;
    contextSettings.antialiasingLevel = 10; // TODO
    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y, 32), "Mondaine Clock", sf::Style::None, contextSettings);
    window.setPosition(sf::Vector2i(
        (sf::VideoMode::getDesktopMode().width  - windowSize.x) / 2,
        (sf::VideoMode::getDesktopMode().height - windowSize.y) / 2));

    // These functions return false on an unsupported OS or when it is not supported on linux (e.g. display doesn't support shape extention)
    setShapeCircle(window.getSystemHandle(), windowSize);
    setTransparency(window.getSystemHandle(), opacity);

    // We will also draw the image on the window instead of just showing an empty window with the wanted shape
    //sf::Texture backgroundTexture;
    //sf::Sprite backgroundSprite;
    //backgroundTexture.loadFromImage(backgroundImage);
    //backgroundSprite.setTexture(backgroundTexture);

    // Main loop to display the image while the window is open (pressing the escape key to close the window)
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || (event.key.code == sf::Keyboard::Escape && event.type == sf::Event::KeyPressed))
                window.close();
        }

        window.clear(sf::Color::Transparent);
        //window.clear(sf::Color(255, 0, 255, 255));

        //auto now = std::chrono::system_clock::now();
        //std::time_t end_time = std::chrono::system_clock::to_time_t(now);
        //auto now2 = std::ctime(&end_time);
        //int hours = now2.

        //time_t currentTime;
        //struct tm* localTime = nullptr;
        //
        //time(&currentTime);                   // Get the current time
        ////localTime = localtime(&currentTime);  // Convert the current time to the local time
        //localtime_s(localTime, &currentTime);
        //
        //int Hour = localTime->tm_hour;
        //int Min = localTime->tm_min;
        //int Sec = localTime->tm_sec;

        // get the current time point
        const std::time_t now = std::time(nullptr);
        // convert it to (local) calendar time
        const std::tm calendar_time = *std::localtime(std::addressof(now));

        int Hour = calendar_time.tm_hour;
        int Min  = calendar_time.tm_min;
        int Sec  = calendar_time.tm_sec;

        DrawMondaineClock(window, Hour, Min, Sec);
        //backgroundSprite.setColor(sf::Color(255, 255, 255, 100));
        //window.draw(backgroundSprite);
        window.display();
    }

    return 0;
}
