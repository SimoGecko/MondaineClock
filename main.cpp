#include <SFML/Graphics.hpp>
#include <cstring>
#include <ctime>
#include <windows.h>

// PARAMETERS
int csize = 200;
int posx = -1;
int posy = -1;
bool stop2go = true;
bool dark = false;
unsigned int handcolor = 0;
float secondPeriod = 58.f;

// state
bool isMouseDragging = false;
int lastDownX = 0, lastDownY = 0;

// CLOCK
#define scaling ((float)csize/360)
#define U *scaling

#define clock_r  (180 U)
#define clock_b  ( 12 U)

#define ticks_r  (160 U)
#define ticks_l1 ( 40 U)
#define ticks_w1 ( 12 U)
#define ticks_l2 ( 12 U)
#define ticks_w2 (  5 U)

#define hour_l1  ( 94 U)
#define hour_l2  ( 39 U)
#define hour_w1  ( 15 U)
#define hour_w2  ( 21 U)

#define min_l1   (152 U)
#define min_l2   ( 40 U)
#define min_w1   ( 12 U)
#define min_w2   ( 18 U)

#define sec_l1   (110 U)
#define sec_l2   ( 52 U)
#define sec_w    (  5 U)
#define sec_r    ( 13 U)

#define col_white 0xfdfdfd
#define col_black 0x221e20
#define col_red   0xec2324

#define TO_RGBA(hex) ((hex)<<8 | 0xff)
#define POW2(x) ((x)*(x))

////////////////////////////////////////////////// SFML //////////////////////////////////////////////////

static sf::CircleShape getCircleShape(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, float radius, int segments = 32, sf::Color outlineColor = sf::Color::Transparent, float outlineThickness = 0.f)
{
    sf::CircleShape cs(radius, segments);
    cs.setFillColor(color);
    cs.setOutlineColor(outlineColor);
    cs.setOutlineThickness(outlineThickness);
    cs.setOrigin(origin);
    cs.setPosition(position);
    cs.setRotation(sf::degrees(rotation));
    return cs;
}
static sf::RectangleShape getRectangleShape (sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, sf::Vector2f size)
{
    sf::RectangleShape rs;
    rs.setFillColor(color);
    rs.setOrigin(origin);
    rs.setPosition(position);
    rs.setRotation(sf::degrees(rotation));
    rs.setSize(size);
    return rs;
}
static sf::ConvexShape getTrapezoidShape    (sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, sf::Vector3f size)
{
    //sf::Vector2f approxSize((size.x + size.y) / 2.f, size.z);
    //return GetRectangle(color, origin, position, rotation, approxSize);

    sf::ConvexShape xs;
    const float d = (size.y - size.x) / 2.f;
    xs.setPointCount(4);
    xs.setPoint(0, sf::Vector2f(0.f, 0.f));
    xs.setPoint(1, sf::Vector2f(size.x, 0.f));
    xs.setPoint(2, sf::Vector2f(size.x + d, size.z));
    xs.setPoint(3, sf::Vector2f(-d, size.z));

    xs.setFillColor(color);
    xs.setOrigin(origin);
    xs.setPosition(position);
    xs.setRotation(sf::degrees(rotation));
    return xs;
}

static void drawClockBackground(sf::RenderTarget& target)
{
    const sf::Color white(TO_RGBA(!dark ? col_white : col_black));
    const sf::Color black(TO_RGBA(!dark ? col_black : col_white));
    const sf::Vector2f center(clock_r, clock_r);

    // draw background + border
    sf::Vector2f origin(clock_r, clock_r);
    target.draw(getCircleShape(white, origin, center, 0.f, clock_r, 120, black, -clock_b));

    // draw ticks
    for (int i = 0; i < 60; ++i)
    {
        bool largeTick = (i % 5 == 0);
        sf::Vector2f size = largeTick ? sf::Vector2f(ticks_w1, ticks_l1) : sf::Vector2f(ticks_w2, ticks_l2);
        float angle = (float)i * 6.f; // / 60.f * 360.f;
        sf::Vector2f origin(size.x / 2.f, ticks_r);
        target.draw(getRectangleShape(black, origin, center, angle, size));
    }
}

static void createClockHands(sf::Shape& hShape, sf::Shape& mShape, sf::Shape& s1Shape, sf::Shape& s2Shape)
{
    const sf::Color black(TO_RGBA(!dark ? col_black : col_white));
    const sf::Color red(TO_RGBA(handcolor == 0 ? col_red : handcolor));
    const sf::Vector2f center(clock_r, clock_r);

    const float angle = 0.f;

    // create hours
    {
        sf::Vector3f size(hour_w1, hour_w2, hour_l1 + hour_l2);
        sf::Vector2f origin(size.x / 2.f, hour_l1);
        hShape = getTrapezoidShape(black, origin, center, angle, size);
    }

    // create minutes
    {
        sf::Vector3f size(min_w1, min_w2, min_l1 + min_l2);
        sf::Vector2f origin(size.x / 2.f, min_l1);
        mShape = getTrapezoidShape(black, origin, center, angle, size);
    }

    // create seconds
    {
        sf::Vector2f size(sec_w, sec_l1 + sec_l2);
        sf::Vector2f origin1(size.x / 2.f, sec_l1);
        s1Shape = getRectangleShape(red, origin1, center, angle, size);

        sf::Vector2f origin2(sec_r, sec_r + sec_l1);
        float radius = sec_r;
        s2Shape = getCircleShape(red, origin2, center, angle, radius, 32);
    }
}

static void drawClockHands(sf::RenderTarget& target, sf::Shape& hShape, sf::Shape& mShape, sf::Shape& s1Shape, sf::Shape& s2Shape, int h, int m, int s)
{
    // draw hours
    float hAngle = (float)(h % 12) * 30.f + (float)(m % 60) * 0.5f;
    hShape.setRotation(sf::degrees(hAngle));
    target.draw(hShape);

    // draw minutes
    float mAngle = (float)(m % 60) * 6.f;
    mShape.setRotation(sf::degrees(mAngle));
    target.draw(mShape);

    // draw seconds
    float sAngle = (float)(s % 60) * 6.f;
    s1Shape.setRotation(sf::degrees(sAngle));
    s2Shape.setRotation(sf::degrees(sAngle));
    target.draw(s1Shape);
    target.draw(s2Shape);
}

static void processEvents(sf::Window& window)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())// || (event->is<sf::Event::KeyPressed>() && event.key.code == sf::Keyboard::Escape))
        {
            window.close();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
                window.close();
        }
        // drag window
        else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            // TODO: main mouse only
            isMouseDragging = true;
            lastDownX = mousePressed->position.x;
            lastDownY = mousePressed->position.y;
        }
        else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
        {
            if (isMouseDragging)
            {
                window.setPosition(window.getPosition() + sf::Vector2<int>(mouseMoved->position.x - lastDownX, mouseMoved->position.y - lastDownY));
            }
        }
        else if (event->is<sf::Event::MouseButtonReleased>())
        {
            isMouseDragging = false;
        }
    }
}

////////////////////////////////////////////////// WINDOWS //////////////////////////////////////////////////

bool setWindowShapeCircle(HWND hWnd, const sf::Vector2u& size)
{
    HRGN hRegion = CreateRectRgn(0, 0, size.x, size.y);

    // Determine the visible region
    int d2 = POW2(size.x / 2);
    for (int y = 0; y < size.y; y++)
    {
        for (int x = 0; x < size.x; x++)
        {
            int d = POW2(x - size.x / 2) + POW2(y - size.y / 2);
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

bool setWindowTransparency(HWND hWnd, unsigned char alpha)
{
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
    return true;
}

////////////////////////////////////////////////// SYSTEM //////////////////////////////////////////////////

static void getTime(int& Hour, int& Min, int& Sec)
{
    //const std::time_t now = std::time(nullptr);
    //const std::tm calendar_time = *std::localtime(std::addressof(now));

    SYSTEMTIME st;
    GetLocalTime(&st);

    Hour = st.wHour;   //calendar_time.tm_hour;
    Min  = st.wMinute;
    Sec  = st.wSecond;
    if (stop2go)
    {
        int Msec = st.wMilliseconds;
        float Secf = (float)Sec + (float)Msec / 1000.f;
        Secf = Secf * (60.f / secondPeriod);
        Secf = Secf < 0.f ? 0.f : Secf > 60.f ? 60.f : Secf;
        Sec = (int)Secf; // floor
    }
}

static int getWaitTimeMs()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    int Msec = st.wMilliseconds;

    if (stop2go)
    {
        int Sec = st.wSecond;
        if (Sec >= secondPeriod) return 1000 - Msec;

        int tot = Sec * 1000 + Msec;
        // find next multiple of 966.66=1000*58/60
        int next = ((tot / 967) + 1) * 967;
        return next - tot;
    }
    else
    {
        return 1000 - Msec;
    }
}

static void readParams(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        if      (_strcmpi(argv[i], "size")        == 0 && (i + 1 < argc)) csize       = std::stoi(argv[++i]);
        else if (_strcmpi(argv[i], "posx")        == 0 && (i + 1 < argc)) posx        = std::stoi(argv[++i]);
        else if (_strcmpi(argv[i], "posy")        == 0 && (i + 1 < argc)) posy        = std::stoi(argv[++i]);
        else if (_strcmpi(argv[i], "stop2go")     == 0 && (i + 1 < argc)) stop2go     = (_strcmpi(argv[++i], "0") != 0);
        else if (_strcmpi(argv[i], "dark")        == 0                  ) dark        = true;
        else if (_strcmpi(argv[i], "handcolor")   == 0 && (i + 1 < argc))
        {
            int hex = std::stoi(argv[++i], 0, 16);
            handcolor = ((hex << 8) | 0xff);
        }
    }
}

int main(int argc, char** argv)
{
    readParams(argc, argv);

    sf::Vector2u windowSize(csize, csize);
    const int windowsTaskbarHeight = 48 * 1.5;
    if (posx == -1) posx = (sf::VideoMode::getDesktopMode().size.x - windowSize.x) - 10;
    if (posy == -1) posy = (sf::VideoMode::getDesktopMode().size.y - windowSize.y) - 10 - windowsTaskbarHeight;
    sf::Vector2i windowPosition(posx, posy);

    // Create the window
    sf::ContextSettings contextSettings;
    contextSettings.antiAliasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(windowSize, 32), sf::String("Mondaine Clock"), sf::Style::None, sf::State::Windowed, contextSettings);
    window.setPosition(windowPosition);
    window.setFramerateLimit(5);

    setWindowShapeCircle(window.getNativeHandle(), windowSize);
    //setTransparency(window.getSystemHandle(), 255);
    //ShowWindow(window.getSystemHandle(), SW_HIDE);

    // prepare the background
    sf::RenderTexture renderTexture(windowSize, contextSettings);
    renderTexture.clear(sf::Color(TO_RGBA(col_black)));
    drawClockBackground(renderTexture);
    renderTexture.display();
    const sf::Texture& textureBg = renderTexture.getTexture();
    sf::Sprite spriteBg(textureBg);

    // pre-create the hand shapes
    sf::RectangleShape hShape, mShape, s1Shape; // HACK: it works to assign a sf::ConvexShape with 4 vertices to a sf::RectangleShape
    sf::CircleShape s2Shape;
    createClockHands(hShape, mShape, s1Shape, s2Shape);

    while (window.isOpen())
    {
        processEvents(window);

        int Hour, Min, Sec;
        getTime(Hour, Min, Sec);
        //Hour = 10; Min = 9; Sec = 37; // special stock time

        window.draw(spriteBg); // clear
        drawClockHands(window, hShape, mShape, s1Shape, s2Shape, Hour, Min, Sec);
        window.display();

        sf::sleep(sf::milliseconds(getWaitTimeMs()));
    }

    return 0;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    return main(__argc, __argv);
}