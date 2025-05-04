#version 330

uniform vec2  u_center;      /** Kör középpont ablakkoordinátákban */
uniform float u_radius;      /** Kör sugara pixelben */
uniform float u_halfSeg;     /** Szakasz félhossza NDC-ben */
uniform float u_lineThick;   /** Szakasz vastagsága pixelben */

out vec4 fragColor;          /** Kimeneti színvektor */

void main() {
    vec2 P = gl_FragCoord.xy;                         /** Aktuális fragment pozíció pixelben */
    float d = distance(P, u_center);                  /** Távolság a kör középpontjától */

    if (d <= u_radius) {                              /** Ha a fragment a körön belül van */
        float t = d / u_radius;                       /** Normalizált távolság [0,1] */
        vec3 c = mix(vec3(1,0,0), vec3(0,1,0), t);    /** Piros+zöld lineáris interpoláció */
        fragColor = vec4(c, 1.0);                    /** RGBA szín beállítása */
        return;                                      /** Kilépünk, no discard */
    }

    float dy = abs(P.y - u_center.y);                 /** Függőleges eltérés a középtől */
    float dx = abs(P.x - u_center.x);                 /** Vízszintes eltérés a középtől */

    if (dy <= u_lineThick * 0.5 && dx <= u_halfSeg) { /** Ha a szakasz régiójában vagyunk */
        fragColor = vec4(0,0,1,1);                    /** Kék szín */
        return;                                      /** Kilépünk */
    }

    discard;                                          /** Minden más fragment discard */
}
