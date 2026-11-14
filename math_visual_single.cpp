// =============================================================================
// Math Visual Animation - Édition Single-File
// Auteur: HackerStChad
// =============================================================================
//
// Compilation:
//   g++ -std=c++17 -O3 -march=native -ffast-math -o math_visual_single \
//       math_visual_single.cpp -lsfml-graphics -lsfml-window -lsfml-system -lpthread
//
// Exécution:
//   ./math_visual_single
//
// Contrôles:
//   F1-F5 : changer de mode
//   Espace: pause/reprendre
//   R     : réinitialiser la caméra
//   S     : capture d'écran
//   +/-   : zoom
//   Flèches: déplacer la caméra
//   Molette: zoom
//   Échap : quitter
//
// =============================================================================

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <complex>
#include <functional>
#include <chrono>
#include <thread>

// ============================================================================
// Constantes et utilitaires
// ============================================================================

namespace mv {

constexpr float PI       = 3.14159265358979323846f;
constexpr float TWO_PI   = 6.28318530717958647692f;
constexpr float HALF_PI  = 1.57079632679489661923f;
constexpr float EPSILON  = 1e-6f;
constexpr float DEG2RAD  = PI / 180.0f;
constexpr float RAD2DEG  = 180.0f / PI;

inline float clamp(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float remap(float v, float lo1, float hi1, float lo2, float hi2) {
    return lo2 + (v - lo1) * (hi2 - lo2) / (hi1 - lo1);
}

inline sf::Color hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r, g, b;
    if      (h < 60.0f)  { r = c; g = x; b = 0; }
    else if (h < 120.0f) { r = x; g = c; b = 0; }
    else if (h < 180.0f) { r = 0; g = c; b = x; }
    else if (h < 240.0f) { r = 0; g = x; b = c; }
    else if (h < 300.0f) { r = x; g = 0; b = c; }
    else                 { r = c; g = 0; b = x; }
    return sf::Color(
        static_cast<sf::Uint8>((r + m) * 255),
        static_cast<sf::Uint8>((g + m) * 255),
        static_cast<sf::Uint8>((b + m) * 255)
    );
}

inline float randomFloat(float min = 0.0f, float max = 1.0f) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

inline int randomInt(int min, int max) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

inline std::string formatFloat(float value, int decimals = 2) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(decimals) << value;
    return ss.str();
}

inline float sinc(float x) {
    return std::fabs(x) < EPSILON ? 1.0f : std::sin(x) / x;
}

inline float sawtooth(float x) {
    return 2.0f * (x / TWO_PI - std::floor(x / TWO_PI + 0.5f));
}

inline float squareWave(float x) {
    return std::sin(x) >= 0.0f ? 1.0f : -1.0f;
}

inline float triangleWave(float x) {
    return 2.0f * std::asin(std::sin(x)) / PI;
}

inline float pulseWave(float x, float duty = 0.5f) {
    float phase = std::fmod(x, TWO_PI) / TWO_PI;
    return phase < duty ? 1.0f : -1.0f;
}

inline sf::Vector2f worldToScreen(const sf::Vector2f& world, const sf::Vector2f& center, float zoom) {
    return center + world * zoom;
}

inline sf::Vector2f screenToWorld(const sf::Vector2f& screen, const sf::Vector2f& center, float zoom) {
    return (screen - center) / zoom;
}

} // namespace mv

using namespace mv;

// ============================================================================
// Palette de couleurs
// ============================================================================

enum class Palette {
    Rainbow, Inferno, Plasma, Viridis, Magma, Ocean, Heat, Cool, Twilight, Neon
};

sf::Color fromPalette(float t, Palette palette) {
    t = clamp(t, 0.0f, 1.0f);
    switch (palette) {
        case Palette::Rainbow:  return hsvToRgb(t * 360.0f, 1.0f, 1.0f);
        case Palette::Inferno:  {
            sf::Color stops[] = {sf::Color(0,0,4), sf::Color(40,11,84), sf::Color(101,21,110), sf::Color(159,42,99), sf::Color(212,72,66), sf::Color(245,125,21), sf::Color(250,193,39), sf::Color(252,255,164)};
            int n = sizeof(stops)/sizeof(stops[0]) - 1;
            float s = t * n; int i = static_cast<int>(s); float f = s - i;
            sf::Color a = stops[i], b = stops[std::min(i+1,n)];
            return sf::Color(lerp(a.r,b.r,f), lerp(a.g,b.g,f), lerp(a.b,b.b,f));
        }
        case Palette::Plasma:   {
            sf::Color stops[] = {sf::Color(13,8,135), sf::Color(84,2,163), sf::Color(139,10,165), sf::Color(185,50,137), sf::Color(219,92,104), sf::Color(244,136,73), sf::Color(253,187,132), sf::Color(240,249,33)};
            int n = sizeof(stops)/sizeof(stops[0]) - 1;
            float s = t * n; int i = static_cast<int>(s); float f = s - i;
            sf::Color a = stops[i], b = stops[std::min(i+1,n)];
            return sf::Color(lerp(a.r,b.r,f), lerp(a.g,b.g,f), lerp(a.b,b.b,f));
        }
        case Palette::Viridis:  {
            sf::Color stops[] = {sf::Color(68,1,84), sf::Color(72,40,120), sf::Color(62,73,137), sf::Color(49,104,142), sf::Color(38,130,142), sf::Color(31,158,137), sf::Color(53,183,121), sf::Color(109,206,89), sf::Color(180,222,44), sf::Color(253,231,37)};
            int n = sizeof(stops)/sizeof(stops[0]) - 1;
            float s = t * n; int i = static_cast<int>(s); float f = s - i;
            sf::Color a = stops[i], b = stops[std::min(i+1,n)];
            return sf::Color(lerp(a.r,b.r,f), lerp(a.g,b.g,f), lerp(a.b,b.b,f));
        }
        case Palette::Magma:    {
            sf::Color stops[] = {sf::Color(0,0,4), sf::Color(28,16,68), sf::Color(79,18,123), sf::Color(129,37,129), sf::Color(181,54,122), sf::Color(229,80,100), sf::Color(251,135,97), sf::Color(254,194,135), sf::Color(252,253,191)};
            int n = sizeof(stops)/sizeof(stops[0]) - 1;
            float s = t * n; int i = static_cast<int>(s); float f = s - i;
            sf::Color a = stops[i], b = stops[std::min(i+1,n)];
            return sf::Color(lerp(a.r,b.r,f), lerp(a.g,b.g,f), lerp(a.b,b.b,f));
        }
        case Palette::Ocean:    return hsvToRgb(lerp(200.0f, 260.0f, t), 0.8f, lerp(0.2f, 1.0f, t));
        case Palette::Heat:     return sf::Color(static_cast<sf::Uint8>(255*t), static_cast<sf::Uint8>(255*t*t*0.5f), 0);
        case Palette::Cool:     return sf::Color(static_cast<sf::Uint8>(255*t*t), 0, static_cast<sf::Uint8>(255*(1-t)*(1-t)));
        case Palette::Twilight: return hsvToRgb(lerp(260.0f, 340.0f, t), 0.7f, lerp(0.15f, 0.95f, t));
        case Palette::Neon:     return hsvToRgb(lerp(280.0f, 360.0f, std::fmod(t*3.0f,1.0f)), 1.0f, 1.0f);
    }
    return sf::Color::White;
}

sf::Color fractalColor(int iterations, int maxIter, float smoothVal) {
    if (iterations >= maxIter) return sf::Color::Black;
    float t = (iterations + smoothVal) / static_cast<float>(maxIter);
    return fromPalette(std::pow(t, 0.5f), Palette::Inferno);
}

// ============================================================================
// Énumérations
// ============================================================================

enum class SceneMode { Trigonometry, Fourier, Parametric, Fractals, Particles, Mixed };
enum class FunctionType { Sine, Cosine, Tangent, Secant, Cosecant, Cotangent, DampedSine, BeatingWave, Chirp, Gaussian, Sinc, Custom };
enum class CurveType { Lissajous, Epicycloid, Hypocycloid, Rose, Spiral, Butterfly, Dragon, Hilbert, Trochoid, Rhodonea, Nephroid, Astroid, Cardioid, Limacon };
enum class ParticleField { Vortex, Wave, Spiral, Lorenz, Fluid, Gravity, Electromagnetic, NoiseField };
enum class FractalType { Mandelbrot, Julia, BurningShip, Newton, Multibrot, Phoenix, Tricorn };
enum class Waveform { Square, Sawtooth, Triangle, Pulse };

// ============================================================================
// Caméra
// ============================================================================

struct Camera {
    sf::Vector2f center;
    float zoom;
    float rotation;
    float width, height;
    sf::Vector2f velocity;

    Camera(float w, float h) : center(w/2.0f, h/2.0f), zoom(80.0f), rotation(0.0f), width(w), height(h), velocity(0,0) {}

    sf::Vector2f worldToScreen(const sf::Vector2f& world) const {
        return center + world * zoom;
    }
    sf::Vector2f screenToWorld(const sf::Vector2f& screen) const {
        return (screen - center) / zoom;
    }
    sf::Vector2f project3D(const sf::Vector3f& v) const {
        float s = 1.0f / (1.0f + v.z * 0.003f);
        return center + sf::Vector2f(v.x * s, v.y * s) * zoom;
    }
    void pan(float dx, float dy) { center.x += dx; center.y += dy; }
    void doZoom(float factor, sf::Vector2f anchor) {
        sf::Vector2f worldBefore = screenToWorld(anchor);
        zoom = clamp(zoom * factor, 0.01f, 10000.0f);
        sf::Vector2f worldAfter = screenToWorld(anchor);
        center += (worldAfter - worldBefore) * zoom;
    }
    void reset(float w, float h) { center = sf::Vector2f(w/2.0f, h/2.0f); zoom = 80.0f; rotation = 0.0f; }
};

// ============================================================================
// Moteur mathématique
// ============================================================================

float evaluateWave(FunctionType type, float x, float t, float amplitude = 1.0f, float frequency = 1.0f, float phase = 0.0f) {
    float angle = x * frequency + t + phase;
    switch (type) {
        case FunctionType::Sine:        return amplitude * std::sin(angle);
        case FunctionType::Cosine:      return amplitude * std::cos(angle);
        case FunctionType::Tangent:     return amplitude * std::tan(angle);
        case FunctionType::Secant:      return amplitude / (std::cos(angle) + EPSILON);
        case FunctionType::Cosecant:    return amplitude / (std::sin(angle) + EPSILON);
        case FunctionType::Cotangent:   return amplitude / (std::tan(angle) + EPSILON);
        case FunctionType::DampedSine:  return amplitude * std::exp(-0.1f*x*x) * std::sin(angle);
        case FunctionType::BeatingWave: return amplitude * std::sin(angle) * std::cos(angle * 0.1f);
        case FunctionType::Chirp:       return amplitude * std::sin(angle + 0.5f*t*x*x);
        case FunctionType::Gaussian:    return amplitude * std::exp(-x*x*frequency) * std::cos(angle);
        case FunctionType::Sinc:        return amplitude * sinc(angle);
        default:                        return amplitude * std::sin(angle);
    }
}

float fourierSum(float x, float t, int harmonics, Waveform waveform) {
    float sum = 0.0f;
    for (int n = 1; n <= harmonics; ++n) {
        float amp = 0.0f;
        switch (waveform) {
            case Waveform::Square:    amp = (n % 2 == 1) ? 4.0f / (PI * n) : 0.0f; break;
            case Waveform::Sawtooth:  amp = 2.0f * std::pow(-1.0f, n+1) / (PI * n); break;
            case Waveform::Triangle:  amp = (n % 2 == 1) ? 8.0f * std::pow(-1.0f, (n-1)/2) / (PI * PI * n * n) : 0.0f; break;
            case Waveform::Pulse:     amp = std::sin(PI * n * 0.3f) / (PI * n); break;
        }
        sum += amp * std::sin(n * x + t);
    }
    return sum;
}

// ============================================================================
// Courbes paramétriques
// ============================================================================

sf::Vector2f evaluateCurve(CurveType type, float t, float a, float b, float c, float d, float scale) {
    switch (type) {
        case CurveType::Lissajous:
            return sf::Vector2f(scale * std::sin(a * t + d), scale * std::sin(b * t));
        case CurveType::Epicycloid: {
            float r = a, R = b;
            return sf::Vector2f(scale * ((R + r) * std::cos(t) - r * std::cos((R + r) / r * t)),
                                scale * ((R + r) * std::sin(t) - r * std::sin((R + r) / r * t)));
        }
        case CurveType::Hypocycloid: {
            float r = a, R = b;
            return sf::Vector2f(scale * ((R - r) * std::cos(t) + r * std::cos((R - r) / r * t)),
                                scale * ((R - r) * std::sin(t) - r * std::sin((R - r) / r * t)));
        }
        case CurveType::Rose:
            return sf::Vector2f(scale * std::cos(a * t) * std::cos(t), scale * std::cos(a * t) * std::sin(t));
        case CurveType::Spiral:
            return sf::Vector2f(scale * t * std::cos(a * t), scale * t * std::sin(a * t));
        case CurveType::Butterfly: {
            float expVal = std::exp(std::cos(t));
            float sinCos = std::sin(t);
            float cos12 = std::cos(12 * t);
            float sin5 = std::sin(t / 12.0f);
            float r = scale * 0.05f * (expVal - 2 * cos12 + sinCos * sinCos * sinCos * sin5);
            return sf::Vector2f(r * std::sin(t), -r * std::cos(t));
        }
        case CurveType::Trochoid: {
            float r = a, d = b;
            return sf::Vector2f(scale * (r * t - d * std::sin(t)), scale * (r - d * std::cos(t)));
        }
        case CurveType::Rhodonea: {
            float r = scale * std::cos(a * t);
            return sf::Vector2f(r * std::cos(t), r * std::sin(t));
        }
        case CurveType::Nephroid: {
            return sf::Vector2f(scale * (3 * std::cos(t) - std::cos(3 * t)),
                                scale * (3 * std::sin(t) - std::sin(3 * t)));
        }
        case CurveType::Astroid: {
            return sf::Vector2f(scale * std::pow(std::cos(t), 3), scale * std::pow(std::sin(t), 3));
        }
        case CurveType::Cardioid: {
            float r = scale * (1 - std::cos(t));
            return sf::Vector2f(r * std::cos(t), r * std::sin(t));
        }
        case CurveType::Limacon: {
            float r = scale * (a + b * std::cos(t));
            return sf::Vector2f(r * std::cos(t), r * std::sin(t));
        }
        default:
            return sf::Vector2f(scale * std::cos(t), scale * std::sin(t));
    }
}

sf::Vector3f evaluateCurve3D(float t, int mode) {
    switch (mode) {
        case 0: return sf::Vector3f(100 * std::cos(t), 100 * std::sin(t), 50 * std::sin(3*t));
        case 1: return sf::Vector3f(80 * std::cos(t) * std::cos(3*t), 80 * std::sin(t), 80 * std::cos(t) * std::sin(3*t));
        case 2: return sf::Vector3f(60 * (std::cos(t) + std::cos(3*t)), 60 * (std::sin(t) - std::sin(3*t)), 40 * std::sin(2*t));
        default: return sf::Vector3f(100 * std::cos(t), 100 * std::sin(t), 0);
    }
}

// ============================================================================
// Fractales
// ============================================================================

struct FractalParams {
    FractalType type;
    int maxIter;
    std::complex<float> juliaC;
    float power;
};

int iterateMandelbrot(std::complex<float> c, int maxIter) {
    std::complex<float> z = 0;
    for (int i = 0; i < maxIter; ++i) {
        if (std::norm(z) > 4.0f) return i;
        z = z * z + c;
    }
    return maxIter;
}

int iterateJulia(std::complex<float> z, std::complex<float> c, int maxIter) {
    for (int i = 0; i < maxIter; ++i) {
        if (std::norm(z) > 4.0f) return i;
        z = z * z + c;
    }
    return maxIter;
}

int iterateBurningShip(std::complex<float> c, int maxIter) {
    std::complex<float> z = 0;
    for (int i = 0; i < maxIter; ++i) {
        if (std::norm(z) > 4.0f) return i;
        z = std::complex<float>(std::fabs(z.real()), std::fabs(z.imag()));
        z = z * z + c;
    }
    return maxIter;
}

int iterateNewton(std::complex<float> z, int maxIter) {
    for (int i = 0; i < maxIter; ++i) {
        std::complex<float> f = z * z * z - 1.0f;
        std::complex<float> df = 3.0f * z * z;
        if (std::norm(df) < EPSILON) return maxIter;
        std::complex<float> zNext = z - f / df;
        if (std::norm(zNext - z) < 1e-6f) return i;
        z = zNext;
    }
    return maxIter;
}

int iterateTricorn(std::complex<float> c, int maxIter) {
    std::complex<float> z = 0;
    for (int i = 0; i < maxIter; ++i) {
        if (std::norm(z) > 4.0f) return i;
        z = std::conj(z) * std::conj(z) + c;
    }
    return maxIter;
}

// ============================================================================
// Particules
// ============================================================================

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float life;
    float maxLife;
    float size;
};

class ParticleSystem {
public:
    std::vector<Particle> particles;
    ParticleField field;
    sf::Vector2f origin;
    float intensity;
    Palette palette;

    ParticleSystem(size_t count = 6000) : field(ParticleField::Vortex), origin(0,0), intensity(1.0f), palette(Palette::Rainbow) {
        particles.resize(count);
        reset();
    }

    void reset() {
        for (auto& p : particles) {
            p.pos = sf::Vector2f(randomFloat(-10,10), randomFloat(-10,10));
            p.vel = sf::Vector2f(randomFloat(-1,1), randomFloat(-1,1));
            p.life = randomFloat(0.5f, 3.0f);
            p.maxLife = p.life;
            p.size = randomFloat(1.0f, 4.0f);
        }
    }

    sf::Vector2f force(const Particle& p, float time) const {
        sf::Vector2f v = p.pos - origin;
        float d = std::sqrt(v.x*v.x + v.y*v.y) + EPSILON;
        switch (field) {
            case ParticleField::Vortex:
                return sf::Vector2f(-v.y / d, v.x / d) * intensity;
            case ParticleField::Wave:
                return sf::Vector2f(std::cos(p.pos.x + time), std::sin(p.pos.y + time)) * intensity;
            case ParticleField::Spiral:
                return sf::Vector2f(-v.y / d + v.x * 0.1f, v.x / d + v.y * 0.1f) * intensity;
            case ParticleField::Lorenz: {
                float sigma = 10.0f, rho = 28.0f, beta = 8.0f/3.0f;
                return sf::Vector2f(sigma * (v.y - v.x), v.x * (rho - 1.0f) - v.y) * intensity * 0.01f;
            }
            case ParticleField::Fluid:
                return sf::Vector2f(std::sin(p.pos.y * 0.5f + time), std::cos(p.pos.x * 0.5f + time)) * intensity;
            case ParticleField::Gravity:
                return sf::Vector2f(-v.x / (d*d), -v.y / (d*d)) * intensity * 10.0f;
            case ParticleField::Electromagnetic: {
                sf::Vector2f e(std::cos(p.pos.x + time), std::sin(p.pos.y + time));
                sf::Vector2f b(-v.y, v.x);
                return (e + sf::Vector2f(p.vel.y * b.z - p.vel.z * b.y, p.vel.z * b.x - p.vel.x * b.z)) * intensity;
            }
            case ParticleField::NoiseField:
                return sf::Vector2f(std::sin(p.pos.x * 0.3f + time) * std::cos(p.pos.y * 0.3f),
                                    std::cos(p.pos.x * 0.3f) * std::sin(p.pos.y * 0.3f + time)) * intensity;
        }
        return sf::Vector2f(0,0);
    }

    void update(float dt, float time) {
        for (auto& p : particles) {
            sf::Vector2f f = force(p, time);
            p.vel += f * dt;
            p.vel *= 0.99f;
            p.pos += p.vel * dt;
            p.life -= dt;
            if (p.life <= 0.0f) {
                p.pos = sf::Vector2f(randomFloat(-10,10), randomFloat(-10,10));
                p.vel = sf::Vector2f(randomFloat(-1,1), randomFloat(-1,1));
                p.life = randomFloat(0.5f, 3.0f);
                p.maxLife = p.life;
            }
        }
    }

    void draw(sf::RenderTarget& target, const Camera& cam) {
        sf::VertexArray va(sf::Points, particles.size());
        for (size_t i = 0; i < particles.size(); ++i) {
            const auto& p = particles[i];
            float t = 1.0f - p.life / p.maxLife;
            va[i].position = cam.worldToScreen(p.pos);
            va[i].color = fromPalette(t, palette);
            va[i].color.a = static_cast<sf::Uint8>(255 * (p.life / p.maxLife));
        }
        target.draw(va);
    }
};

// ============================================================================
// Rendu d'axes et grille
// ============================================================================

void drawGrid(sf::RenderTarget& target, const Camera& cam, float alpha = 30) {
    sf::Color gridColor(80, 80, 80, alpha);
    sf::Color axisColor(180, 180, 180, alpha + 60);

    sf::Vector2f tl = cam.screenToWorld(sf::Vector2f(0, 0));
    sf::Vector2f br = cam.screenToWorld(sf::Vector2f(cam.width, cam.height));

    float step = std::pow(10.0f, std::floor(std::log10(100.0f / cam.zoom)));
    if (step < 0.001f) step = 0.001f;

    float startX = std::floor(tl.x / step) * step;
    float endX = std::ceil(br.x / step) * step;
    float startY = std::floor(br.y / step) * step;
    float endY = std::ceil(tl.y / step) * step;

    sf::VertexArray lines(sf::Lines);
    for (float x = startX; x <= endX; x += step) {
        sf::Vector2f p1 = cam.worldToScreen(sf::Vector2f(x, startY));
        sf::Vector2f p2 = cam.worldToScreen(sf::Vector2f(x, endY));
        lines.append(sf::Vertex(p1, gridColor));
        lines.append(sf::Vertex(p2, gridColor));
    }
    for (float y = startY; y <= endY; y += step) {
        sf::Vector2f p1 = cam.worldToScreen(sf::Vector2f(startX, y));
        sf::Vector2f p2 = cam.worldToScreen(sf::Vector2f(endX, y));
        lines.append(sf::Vertex(p1, gridColor));
        lines.append(sf::Vertex(p2, gridColor));
    }
    target.draw(lines);

    sf::VertexArray axes(sf::Lines);
    sf::Vector2f ox1 = cam.worldToScreen(sf::Vector2f(startX, 0));
    sf::Vector2f ox2 = cam.worldToScreen(sf::Vector2f(endX, 0));
    sf::Vector2f oy1 = cam.worldToScreen(sf::Vector2f(0, startY));
    sf::Vector2f oy2 = cam.worldToScreen(sf::Vector2f(0, endY));
    axes.append(sf::Vertex(ox1, axisColor));
    axes.append(sf::Vertex(ox2, axisColor));
    axes.append(sf::Vertex(oy1, axisColor));
    axes.append(sf::Vertex(oy2, axisColor));
    target.draw(axes);
}

// ============================================================================
// Modes de rendu
// ============================================================================

void drawTrigFunctions(sf::RenderTarget& target, const Camera& cam, float time) {
    std::vector<FunctionType> types = {
        FunctionType::Sine, FunctionType::Cosine, FunctionType::Tangent,
        FunctionType::Secant, FunctionType::DampedSine, FunctionType::Chirp
    };
    std::vector<std::string> names = {"sin", "cos", "tan", "sec", "damped", "chirp"};

    sf::Vector2f tl = cam.screenToWorld(sf::Vector2f(0, 0));
    sf::Vector2f br = cam.screenToWorld(sf::Vector2f(cam.width, cam.height));
    float startX = tl.x - 2.0f;
    float endX = br.x + 2.0f;
    int segments = 2000;
    float dx = (endX - startX) / segments;

    for (size_t k = 0; k < types.size(); ++k) {
        sf::VertexArray va(sf::LineStrip, segments + 1);
        float offset = (static_cast<float>(k) - types.size()/2.0f) * 0.5f;
        for (int i = 0; i <= segments; ++i) {
            float x = startX + i * dx;
            float y = evaluateWave(types[k], x, time, 1.0f, 1.0f + k*0.1f, offset) + offset * 2.0f;
            va[i].position = cam.worldToScreen(sf::Vector2f(x, y));
            va[i].color = fromPalette(static_cast<float>(k)/types.size() + time*0.05f, Palette::Rainbow);
        }
        target.draw(va);
    }
}

void drawFourier(sf::RenderTarget& target, const Camera& cam, float time) {
    sf::Vector2f center = cam.worldToScreen(sf::Vector2f(0,0));
    sf::Vector2f pos = center;
    float radiusScale = 60.0f;

    std::vector<Waveform> waveforms = {Waveform::Square, Waveform::Sawtooth, Waveform::Triangle, Waveform::Pulse};
    static int wfIndex = 0;
    Waveform wf = waveforms[wfIndex % waveforms.size()];
    int harmonics = 20;

    sf::VertexArray circles(sf::Lines);
    for (int n = 1; n <= harmonics; ++n) {
        float amp = 0.0f;
        switch (wf) {
            case Waveform::Square:    amp = (n % 2 == 1) ? 4.0f / (PI * n) : 0.0f; break;
            case Waveform::Sawtooth:  amp = 2.0f * std::pow(-1.0f, n+1) / (PI * n); break;
            case Waveform::Triangle:  amp = (n % 2 == 1) ? 8.0f * std::pow(-1.0f, (n-1)/2) / (PI * PI * n * n) : 0.0f; break;
            case Waveform::Pulse:     amp = std::sin(PI * n * 0.3f) / (PI * n); break;
        }
        float radius = radiusScale * std::fabs(amp);
        float angle = n * time + (n % 2 == 0 ? PI : 0);
        sf::Vector2f next = pos + radius * sf::Vector2f(std::cos(angle), std::sin(angle));

        for (int i = 0; i < 32; ++i) {
            float a1 = i / 32.0f * TWO_PI;
            float a2 = (i+1) / 32.0f * TWO_PI;
            sf::Color col = fromPalette(static_cast<float>(n)/harmonics, Palette::Plasma);
            col.a = 80;
            circles.append(sf::Vertex(pos + radius * sf::Vector2f(std::cos(a1), std::sin(a1)), col));
            circles.append(sf::Vertex(pos + radius * sf::Vector2f(std::cos(a2), std::sin(a2)), col));
        }
        circles.append(sf::Vertex(pos, sf::Color(200,200,200,100)));
        circles.append(sf::Vertex(next, sf::Color(200,200,200,100)));
        pos = next;
    }
    target.draw(circles);

    sf::VertexArray wave(sf::LineStrip);
    float waveX = cam.width * 0.25f;
    for (int i = 0; i < 400; ++i) {
        float x = i * 0.01f;
        float y = fourierSum(x * 3.0f, time, harmonics, wf);
        wave.append(sf::Vertex(sf::Vector2f(waveX + i * 2.0f, center.y - y * radiusScale), fromPalette(i/400.0f, Palette::Viridis)));
    }
    target.draw(wave);
}

void drawParametric(sf::RenderTarget& target, const Camera& cam, float time) {
    std::vector<CurveType> types = {CurveType::Lissajous, CurveType::Epicycloid, CurveType::Hypocycloid,
                                    CurveType::Rose, CurveType::Butterfly, CurveType::Spiral};
    for (size_t k = 0; k < types.size(); ++k) {
        int segments = 1500;
        sf::VertexArray va(sf::LineStrip, segments + 1);
        float a = 3.0f + k * 0.5f;
        float b = 2.0f + k * 0.3f;
        float c = 1.0f;
        float d = time * 0.3f;
        float scale = 2.0f;
        for (int i = 0; i <= segments; ++i) {
            float t = i / static_cast<float>(segments) * TWO_PI * (types[k] == CurveType::Spiral ? 4.0f : 1.0f);
            sf::Vector2f p = evaluateCurve(types[k], t + time * (k+1) * 0.2f, a, b, c, d, scale);
            va[i].position = cam.worldToScreen(p);
            va[i].color = fromPalette(static_cast<float>(i)/segments, Palette::Rainbow);
        }
        target.draw(va);
    }

    // Courbe 3D projetée
    sf::VertexArray curve3d(sf::LineStrip, 800);
    for (int i = 0; i < 800; ++i) {
        float t = i / 800.0f * TWO_PI * 4.0f;
        sf::Vector3f p = evaluateCurve3D(t + time, static_cast<int>(time) % 3);
        curve3d[i].position = cam.project3D(p);
        curve3d[i].color = fromPalette(i/800.0f, Palette::Neon);
    }
    target.draw(curve3d);
}

void drawFractal(sf::RenderTarget& target, const Camera& cam, float time) {
    sf::Vector2u size(static_cast<unsigned int>(cam.width), static_cast<unsigned int>(cam.height));
    sf::Image img;
    img.create(size.x, size.y, sf::Color::Black);

    FractalParams params;
    params.type = static_cast<FractalType>(static_cast<int>(time * 0.1f) % 5);
    params.maxIter = 200;
    params.juliaC = std::complex<float>(-0.7f + 0.2f * std::sin(time), 0.27015f + 0.2f * std::cos(time));
    params.power = 2.0f;

    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < static_cast<int>(size.y); ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            sf::Vector2f world = cam.screenToWorld(sf::Vector2f(x + 0.5f, y + 0.5f));
            std::complex<float> c(world.x, world.y);
            int iter = 0;
            switch (params.type) {
                case FractalType::Mandelbrot: iter = iterateMandelbrot(c, params.maxIter); break;
                case FractalType::Julia:      iter = iterateJulia(c, params.juliaC, params.maxIter); break;
                case FractalType::BurningShip:iter = iterateBurningShip(c, params.maxIter); break;
                case FractalType::Newton:     iter = iterateNewton(c, params.maxIter); break;
                case FractalType::Tricorn:    iter = iterateTricorn(c, params.maxIter); break;
                default:                      iter = iterateMandelbrot(c, params.maxIter); break;
            }
            img.setPixel(x, y, fractalColor(iter, params.maxIter, 0.0f));
        }
    }

    sf::Texture tex;
    tex.loadFromImage(img);
    sf::Sprite sprite(tex);
    target.draw(sprite);
}

void drawParticles(ParticleSystem& ps, sf::RenderTarget& target, const Camera& cam, float time, float dt) {
    ps.update(dt, time);
    ps.draw(target, cam);
}

// ============================================================================
// HUD
// ============================================================================

void drawHUD(sf::RenderTarget& target, sf::Font& font, const std::string& modeName, float fps, float time, const Camera& cam) {
    sf::RectangleShape panel(sf::Vector2f(320, 140));
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setOutlineColor(sf::Color(100, 200, 255, 200));
    panel.setOutlineThickness(2);
    panel.setPosition(10, 10);
    target.draw(panel);

    auto drawText = [&](float x, float y, const std::string& text, unsigned int size, const sf::Color& color) {
        sf::Text t;
        t.setFont(font);
        t.setString(text);
        t.setCharacterSize(size);
        t.setFillColor(color);
        t.setPosition(x, y);
        target.draw(t);
    };

    drawText(20, 20, "Math Visual Animation", 20, sf::Color(100, 220, 255));
    drawText(20, 48, "Auteur: HackerStChad", 16, sf::Color(200, 200, 200));
    drawText(20, 72, "Mode: " + modeName, 16, sf::Color(255, 255, 150));
    drawText(20, 96, "FPS: " + formatFloat(fps, 1) + " | t=" + formatFloat(time, 2), 14, sf::Color(150, 255, 150));
    drawText(20, 118, "Zoom: " + formatFloat(cam.zoom, 2) + " | Center(" + formatFloat(cam.center.x,1) + ", " + formatFloat(cam.center.y,1) + ")", 14, sf::Color(150, 255, 150));
}

// ============================================================================
// Programme principal
// ============================================================================

int main() {
    const int W = 1280;
    const int H = 720;

    sf::RenderWindow window(sf::VideoMode(W, H), "Math Visual Animation - HackerStChad", sf::Style::Default);
    window.setFramerateLimit(120);
    window.setVerticalSyncEnabled(false);

    Camera cam(W, H);
    ParticleSystem ps(8000);

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    }

    SceneMode mode = SceneMode::Trigonometry;
    float time = 0.0f;
    float speed = 1.0f;
    bool paused = false;
    bool showGrid = true;
    bool showHUD = true;

    sf::Clock clock;
    sf::Clock fpsClock;
    int frameCount = 0;
    float fps = 0.0f;

    bool dragging = false;
    sf::Vector2i lastMouse;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        if (!paused) time += dt * speed;

        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            fps = frameCount / fpsClock.getElapsedTime().asSeconds();
            frameCount = 0;
            fpsClock.restart();
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                cam.width = event.size.width;
                cam.height = event.size.height;
                sf::FloatRect visible(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visible));
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                float factor = event.mouseWheelScroll.delta > 0 ? 1.1f : 0.9f;
                cam.doZoom(factor, sf::Vector2f(sf::Mouse::getPosition(window)));
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Middle || event.mouseButton.button == sf::Mouse::Left) {
                    dragging = true;
                    lastMouse = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle || event.mouseButton.button == sf::Mouse::Left) {
                    dragging = false;
                }
            }
            if (event.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2i pos(event.mouseMove.x, event.mouseMove.y);
                sf::Vector2f delta = sf::Vector2f(pos - lastMouse);
                cam.pan(delta.x, delta.y);
                lastMouse = pos;
            }
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape: window.close(); break;
                    case sf::Keyboard::Space: paused = !paused; break;
                    case sf::Keyboard::R: cam.reset(cam.width, cam.height); break;
                    case sf::Keyboard::G: showGrid = !showGrid; break;
                    case sf::Keyboard::H: showHUD = !showHUD; break;
                    case sf::Keyboard::F1: mode = SceneMode::Trigonometry; break;
                    case sf::Keyboard::F2: mode = SceneMode::Fourier; break;
                    case sf::Keyboard::F3: mode = SceneMode::Parametric; break;
                    case sf::Keyboard::F4: mode = SceneMode::Fractals; break;
                    case sf::Keyboard::F5: mode = SceneMode::Particles; break;
                    case sf::Keyboard::F6: mode = SceneMode::Mixed; break;
                    case sf::Keyboard::Add: case sf::Keyboard::Equal: cam.doZoom(1.2f, sf::Vector2f(cam.width/2, cam.height/2)); break;
                    case sf::Keyboard::Subtract: case sf::Keyboard::Dash: cam.doZoom(0.8f, sf::Vector2f(cam.width/2, cam.height/2)); break;
                    case sf::Keyboard::S: {
                        sf::Texture tex;
                        tex.create(window.getSize().x, window.getSize().y);
                        tex.update(window);
                        sf::Image img = tex.copyToImage();
                        img.saveToFile("screenshot_" + std::to_string(static_cast<int>(time*100)) + ".png");
                        break;
                    }
                    default: break;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  cam.pan(200.0f * dt * cam.zoom, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) cam.pan(-200.0f * dt * cam.zoom, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    cam.pan(0, 200.0f * dt * cam.zoom);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  cam.pan(0, -200.0f * dt * cam.zoom);

        window.clear(sf::Color(10, 10, 18));

        if (showGrid && mode != SceneMode::Fractals) drawGrid(window, cam, 40);

        std::string modeName;
        switch (mode) {
            case SceneMode::Trigonometry: modeName = "Fonctions Trigonometriques"; drawTrigFunctions(window, cam, time); break;
            case SceneMode::Fourier:      modeName = "Series de Fourier"; drawFourier(window, cam, time); break;
            case SceneMode::Parametric:   modeName = "Courbes Parametriques"; drawParametric(window, cam, time); break;
            case SceneMode::Fractals:     modeName = "Fractales"; drawFractal(window, cam, time); break;
            case SceneMode::Particles:    modeName = "Systeme de Particules"; drawParticles(ps, window, cam, time, dt); break;
            case SceneMode::Mixed: {
                modeName = "Mode Mixte";
                drawTrigFunctions(window, cam, time);
                drawParametric(window, cam, time * 0.5f);
                drawParticles(ps, window, cam, time * 1.5f, dt);
                break;
            }
        }

        if (showHUD) drawHUD(window, font, modeName, fps, time, cam);

        window.display();
    }

    return 0;
}
