SPRITE  sprJugador  "monito.png" 4  4   NONE    10
SPRITE  bala  "bala.png"  1   1   FAST    0
SPRITE  malo1  "Enemigo1.png"  4   4   NONE    10
SPRITE  malo2  "Enemigo2.png"  4   4   FAST    0
SPRITE  malo3  "Enemigo3.png"  4   4   NONE    10
SPRITE  moneda  "Moneda.png"  4   4   NONE    10
SPRITE  puerta  "Puerta.png"  2   4   FAST    0

IMAGE background1 "nivel1scrn.png" FAST
IMAGE background2 "nivel1_atras.png" FAST
IMAGE background3 "nivel2scrn.png" FAST

TILESET bga_tileset "nivel1scrn.png" BEST ALL
TILESET bgb_tileset "nivel1_atras.png" BEST ALL
TILESET bgc_tileset "nivel2scrn.png" BEST ALL

MAP bga_map "nivel1scrn.png" bga_tileset BEST 0
MAP bgb_map "nivel1_atras.png" bgb_tileset BEST 0
MAP bgc_map "nivel2scrn.png" bgc_tileset BEST 0

WAV sfx_disparo "sonidos/snare1.wav" 5
WAV sfx_matar "sonidos/S1_explode1.wav" 5
WAV sfx_saltar "sonidos/S1_jump.wav" 2
WAV sfx_moneda "sonidos/S1_ring2.wav" 5
WAV sfx_morir "sonidos/S1_die.wav" 5

XGM cancion1 "musica/City Lights (Act 2).vgm" -1
XGM cancion2 "musica/bonus.vgm" -1
