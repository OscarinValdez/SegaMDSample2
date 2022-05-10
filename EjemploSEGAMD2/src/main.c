#include <genesis.h>
#include <resources.h>
#include "niveles.h"
#include <vdp_bg.h>

#define MAX_BALAS   3
#define MAX_ENEMIGOS	10
#define MAX_MONEDAS     30

#define SFX_DISPARO 64
#define SFX_MATAR 65
#define SFX_SALTAR 66
#define SFX_MONEDA 67
#define SFX_MORIR 68


typedef struct {
	int x;
	int y;
	int w;
	int h;
	int velx;
	int vely;
	int salud;
	bool dir;
	bool act;
	Sprite* sprite;
	int type;
} Entidad;

void matarEntidad(Entidad* j){
	j->salud = 0;
	SPR_setVisibility(j->sprite,HIDDEN);
}

void revivirEntidad(Entidad* j){
	j->salud = 1;
	SPR_setVisibility(j->sprite,VISIBLE);
}

//Declarar las orillas de la pantalla
const int ORILLA_IZQ = 0;
const int ORILLA_DER = 1024;
const int ORILLA_ARR = 0;
const int ORILLA_ABJ = 448;

const int MITAD_1RA_PANT = 160;
const int MITAD_ULTIMA_PANT = 1024 - 160;

const int MITAD_ARRIBA_PANT = 112;
const int MITAD_ABAJO_PANT = 448 - 112;

const int pl_altura = 16;
const int pl_anchura = 16;

const int gravedad = 1;

Sprite* jugador;
int plx=16;
int ply=16;
int posx,posy,showx,showy,movx,movy;
int scr_l,scr_r,scr_u,scr_d;
const int max_movy=5;

bool pldir = FALSE;
bool saltando = FALSE;
bool en_piso = FALSE;

int nivel_actual;
int tile_actual;

u16 balasEnPantalla = 0;
u16 enemigosEnPantalla = 0;
Entidad balas[MAX_BALAS];
Entidad enemigos[MAX_ENEMIGOS];
Entidad monedas[MAX_MONEDAS];
int i,j;

int tile_id[2];
int fondo_id[2];

int num_monedas;
char str_monedas[4] = "0";

Map *bga, *bgb;
u16 ind, bgBaseTileIndex[2];

void prepararBalas(){
    Entidad* b = balas;
    for(i = 0; i < MAX_BALAS; i++){
        b->x = 0;
        b->y = -10;
        b->w = 8;
        b->h = 8;
        if (b->sprite==0)
        b->sprite = SPR_addSprite(&bala,balas[i].x,balas[i].y,TILE_ATTR(PAL1,0,FALSE,FALSE));
        b++;
    }
}

void prepararEnemigos(){
    Entidad* e = enemigos;

    for (i=0;i<1792;i++)
    {
        if (nivel_id[nivel_actual][i]==7)
        {
            plx=(i%64)<<4;
            ply=(i>>6)<<4;
        }

        if (nivel_id[nivel_actual][i]>=3 && nivel_id[nivel_actual][i]<=6)
        {
            e->x = (i%64)<<4;
            e->y = (i>>6)<<4;
            e->w = 32;
            e->h = 32;
            e->velx = -1;
            e->vely = 1;
            e->salud = 1;
            e->dir = 0;

            if (e->sprite==0 && nivel_id[nivel_actual][i]==3)
            e->sprite = SPR_addSprite(&puerta,e->x,e->y,TILE_ATTR(PAL1,0,FALSE,TRUE));

            if (e->sprite==0 && nivel_id[nivel_actual][i]==4)
            e->sprite = SPR_addSprite(&malo1,e->x,e->y,TILE_ATTR(PAL1,0,FALSE,TRUE));

            if (e->sprite==0 && nivel_id[nivel_actual][i]==5)
            e->sprite = SPR_addSprite(&malo2,e->x,e->y,TILE_ATTR(PAL1,0,FALSE,TRUE));

            if (e->sprite==0 && nivel_id[nivel_actual][i]==6)
            e->sprite = SPR_addSprite(&malo3,e->x,e->y,TILE_ATTR(PAL1,0,FALSE,TRUE));

            e->type = nivel_id[nivel_actual][i];

            e++;
        }
    }
}

void prepararMonedas(){
    Entidad* m = monedas;

    for(i = 0; i < 1792; i++){
       if (nivel_id[nivel_actual][i]==2)
       {
            m->x = (i%64)<<4;
            m->y = (i>>6)<<4;
            m->salud = 1;

            if (m->sprite==0)
            m->sprite = SPR_addSprite(&moneda,m->x,m->y,TILE_ATTR(PAL1,0,FALSE,TRUE));

            m++;
       }
    }
}

void dispararBala(){
    if( balasEnPantalla < MAX_BALAS )
    {
        Entidad* b;
        //u16 i = 0;
        for(i=0; i<MAX_BALAS; i++)
        {
            b = &balas[i];
            if(b->salud == 0){

                b->x = plx;
                b->y = ply+10;

                revivirEntidad(b);
                b->velx = 5;
                b->dir = pldir;

                SPR_setPosition(b->sprite,b->x,b->y);
                balasEnPantalla++;
                XGM_startPlayPCM(SFX_DISPARO,1,SOUND_PCM_CH2);
                break;
            }
        }
    }
}

void posicionBalas()
{
    Entidad *b;
    for(i = 0; i < MAX_BALAS; i++)
    {
        b = &balas[i];
        if(b->salud > 0)
        {
            b->x += ((b->velx)*(b->dir?1:-1));

            if(b->x >= scr_l && b->x + b->w < scr_r)
            {
                SPR_setPosition(b->sprite,b->x-showx,b->y-showy);
            }
            else
            {
                matarEntidad(b);
                balasEnPantalla--;
            }
        }
    }
}

void posicionEnemigos(){
    Entidad* e;
    for(i = 0; i < MAX_ENEMIGOS; i++){

        e = &enemigos[i];
        if(e->salud > 0)
        {
            //Checar si se aproxima por la pantalla
            if(
               (e->x+e->w) >= scr_l && (e->x < scr_r)
               && (e->y >= ORILLA_ARR) && (e->y+e->h) < ORILLA_ABJ
            )
            e->act=1;
            else e->act=0;

            if (e->act)
            {
                if (e->type == 4)
                {
                    if (e->dir) e->velx = 1;
                    else e->velx = -1;


                    e->x += e->velx;

                    if (e->vely>max_movy) e->vely=max_movy;
                    e->y+=e->vely;
                    e->vely+=gravedad;

                    if (nivel_id[nivel_actual][(((e->y>>4)+2)<<6)+((e->x>>4))]==1 || nivel_id[nivel_actual][(((e->y>>4)+2)<<6)+(((e->x>>4)+1))]==1)
                    {
                        e->y=(e->y>>4)<<4;
                    }

                    if (nivel_id[nivel_actual][((e->y>>4)<<6)+(((e->x>>4)+1))]==1) e->dir=0;
                    if (nivel_id[nivel_actual][((e->y>>4)<<6)+((e->x>>4))]==1) e->dir=1;

                    SPR_setAnim(e->sprite,0);
                    if (e->dir) SPR_setHFlip(e->sprite, FALSE);
                    else SPR_setHFlip(e->sprite, TRUE);
                }

                if (e->type == 5)
                {
                    if (e->vely>max_movy) e->vely=max_movy;
                    e->y+=e->vely;
                    e->vely+=gravedad;

                    if (nivel_id[nivel_actual][(((e->y>>4)+2)<<6)+((e->x>>4))]==1 || nivel_id[nivel_actual][(((e->y>>4)+2)<<6)+(((e->x>>4)+1))]==1)
                    {
                        e->vely=-10;
                    }
                }
                if (e->type == 6)
                {
                    if (plx<e->x) e->x--; else e->x++;
                    if (ply<e->y) e->y--; else e->y++;
                }

                SPR_setVisibility(e->sprite,VISIBLE);
                SPR_setPosition(e->sprite,e->x-showx,e->y-showy);

                if(
                    e->x <= plx && e->x+16 > plx &&
                    e->y <= ply && e->y+16 > ply
                )
                cargarNivel();
            }
            else
            {
                SPR_setVisibility(e->sprite,HIDDEN);
            }
        }
    }
}

void posicionMonedas(){
    Entidad* m;

     for(i = 0; i < MAX_MONEDAS; i++){
            m = &monedas[i];

            if(m->salud > 0)
            {
                if(
                   (m->x+16) >= scr_l && (m->x < scr_r)
                   && (m->y >= ORILLA_ARR) && (m->y+32) < ORILLA_ABJ
                )
                m->act=1;
                else m->act=0;

                if (m->act)
                {
                    if(
                        m->x <= plx && m->x+16 > plx &&
                        m->y <= ply && m->y+16 > ply
                    )
                    {
                        matarEntidad(m);
                        num_monedas++;
                        XGM_startPlayPCM(SFX_MONEDA,1,SOUND_PCM_CH3);
                    }
                    else
                    {
                        SPR_setVisibility(m->sprite,VISIBLE);
                        SPR_setPosition(m->sprite,m->x-showx,m->y-showy);
                    }
                }
                else
                {
                    SPR_setVisibility(m->sprite,HIDDEN);
                }

            } else SPR_setVisibility(m->sprite,HIDDEN);
     }
}

void prepararControles(u16 control,u16 suelto, u16 boton)
{
    if (control == JOY_1)
	{
		if (boton & BUTTON_RIGHT)
		{
		    pldir = TRUE;
			movx=3;
			SPR_setAnim(jugador,1);
            SPR_setHFlip(jugador, FALSE);
		}
		else if (boton & BUTTON_LEFT)
		{
		    pldir = FALSE;
			movx=-3;
			SPR_setAnim(jugador,1);
            SPR_setHFlip(jugador, TRUE);
		}
        else if( (suelto & BUTTON_RIGHT) | (suelto & BUTTON_LEFT) )
        {
            movx = 0;
            SPR_setAnim(jugador,0);
		}

        if (boton & BUTTON_C)
        {
            if(saltando == FALSE && en_piso)
            {
                saltando = TRUE;
                movy=-13;
                XGM_startPlayPCM(SFX_SALTAR,1,SOUND_PCM_CH4);
            }
        } else if (suelto & BUTTON_C) saltando = FALSE;

        if (boton & BUTTON_B)
        {
            dispararBala();
        }
	}
}

void moverJugador()
{
    if (movy>max_movy) movy=max_movy;
    plx+=movx;
    ply+=movy;

    movy+=gravedad;

    /*Mantener al personaje dentro de la pantalla*/
	if(plx < ORILLA_IZQ) plx = ORILLA_IZQ;
	if(plx + pl_anchura > ORILLA_DER) plx = ORILLA_DER - pl_anchura;

    if (nivel_id[nivel_actual][(((ply>>4)+2)<<6)+((plx+4)>>4)]==1 || nivel_id[nivel_actual][(((ply>>4)+2)<<6)+((plx+11)>>4)]==1)
    {
        ply=(ply>>4)<<4;
        en_piso = TRUE;
    } else en_piso=FALSE;

    if (nivel_id[nivel_actual][(((ply>>4)-1)<<6)+((plx+4)>>4)]==1 || nivel_id[nivel_actual][(((ply>>4)-1)<<6)+((plx+11)>>4)]==1)
    {
        if(movy<0) movy=1;
    }

    if (nivel_id[nivel_actual][((ply>>4)<<6)+(((plx+15)>>4))]==1) plx=(plx>>4)<<4;
    if (nivel_id[nivel_actual][((ply>>4)<<6)+((plx>>4))]==1) plx=((plx>>4)+1)<<4;

    if (ply>ORILLA_ABJ) cargarNivel();

    if (nivel_id[nivel_actual][((ply>>4)<<6)+(plx>>4)]==3)
    {
        nivel_actual=!nivel_actual;
        VDP_resetScreen();
        VDP_clearPlane(BG_B, TRUE);
        plx=0; ply=0;
        MAP_scrollTo(bga, plx-640, ply-640);
        MAP_scrollTo(bgb, plx-640, ply-640);
        reiniciarBalas();
        reiniciarSpritesEnemigos();
        reiniciarMonedas();
        cargarNivel();
    }

    SPR_setPosition(jugador,posx-8,posy);
}

void desplazarPantalla()
{
   if (plx>=MITAD_1RA_PANT && plx <=MITAD_ULTIMA_PANT)
    {
        posx=160;
        MAP_scrollTo(bga, plx<160?0:plx-160, ply<112?0:ply>MITAD_ABAJO_PANT?MITAD_ABAJO_PANT-112:ply-112);
        MAP_scrollTo(bgb, plx<160?0:(plx-160)/2, ply<112?0:ply>MITAD_ABAJO_PANT?(MITAD_ABAJO_PANT-112)/2:(ply-112)/2);
    }
    else if (plx >MITAD_ULTIMA_PANT)
        posx=plx-192;//+(pl_anchura*2);
    else posx=plx;

    if (ply>=MITAD_ARRIBA_PANT && ply <=MITAD_ABAJO_PANT)
    {
        posy=112;
        MAP_scrollTo(bga, plx<160?0:plx>MITAD_ULTIMA_PANT?MITAD_ULTIMA_PANT-160:plx-160, ply-112);
        MAP_scrollTo(bgb, plx<160?0:plx>MITAD_ULTIMA_PANT?(MITAD_ULTIMA_PANT-160)/2:(plx-160)/2, (ply-112)/2);
    }
    else if (ply >MITAD_ABAJO_PANT)
    {
        posy=ply+112+144+(pl_altura*2);
    }
    else
    {
        posy=ply;
    }

}

void checarBalasEnPantalla()
{
    if(balasEnPantalla < MAX_BALAS)
    {
        Entidad* b;

        for(i=0; i<MAX_BALAS; i++)
            {
                b = &balas[i];
            }
    }
}

void checarEnemigosEnPantalla()
{
    if(enemigosEnPantalla < MAX_ENEMIGOS)
    {
        Entidad* e;
        for(i=0; i<MAX_ENEMIGOS; i++)
            {
                e = &enemigos[i];
            }
    }
}

int colisionarEntidades(Entidad* a, Entidad* b)
{
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h >= b->y);
}

void revisarColisionBalasEnemigos()
{
    Entidad* b;
    Entidad* e;

    for(i = 0; i < MAX_BALAS; i++){
        b = &balas[i];
        if(b->salud > 0){
            for(j = 0; j < MAX_ENEMIGOS; j++){
                e = &enemigos[j];
                if(e->salud > 0){
                    if (colisionarEntidades(b,e) && e->type>3){
                        matarEntidad(b);
                        matarEntidad(e);
                        XGM_startPlayPCM(SFX_MATAR,1,SOUND_PCM_CH1);
                        balasEnPantalla--;
                        break;
                    }
                }
            }
        }
    }
}

void reiniciarBalas()
{
    Entidad* b;
    for(i=0; i<MAX_BALAS; i++)
    {
        b = &balas[i];
        b->x=plx;
        b->y=ply;
        b->act =0;
        b->sprite =0;
    }
}

void reiniciarSpritesEnemigos()
{
    Entidad* e;
    for(i=0; i<MAX_ENEMIGOS; i++)
    {
        e = &enemigos[i];
        e->sprite =0;
    }
}

void reiniciarMonedas()
{
    Entidad* m;
    for(i=0; i<MAX_MONEDAS; i++)
    {
        m = &monedas[i];
        m->x=-16;
        m->y=-16;
        m->act =0;
    }
}

void actualizarMonedas(){
	VDP_setTextPalette(7);
	intToStr(num_monedas,str_monedas,1);
	VDP_clearText(0,0,3);
	VDP_drawTextBG(WINDOW,"MONEDAS: ",0,0);
	VDP_drawTextBG(WINDOW,str_monedas,8,0);
}

void cargarNivel()
{
    XGM_setLoopNumber(-1);
    XGM_startPlay(cancion_id[nivel_actual]);

    ind = TILE_USERINDEX;
    bgBaseTileIndex[0] = ind;
    VDP_loadTileSet(mapa_id[nivel_actual], ind, DMA);
    ind += tile_id[nivel_actual];
    bgBaseTileIndex[1] = ind;
    VDP_loadTileSet(&bgb_tileset, ind, DMA);
    ind += bgb_tileset.numTile;

    VDP_setPalette(PAL0, fondo_id[nivel_actual]);
    VDP_setPalette(PAL0, background2.palette->data);

    //VDP_drawText("HOLA MUNDO",1,1);
    VDP_setPaletteColor(PAL0,RGB24_TO_VDPCOLOR(0x6dc2ca));
    VDP_setPalette(PAL1, sprJugador.palette->data);

    bga = MAP_create(mapa2_id[nivel_actual], BG_A, TILE_ATTR_FULL(0, FALSE, FALSE, FALSE, bgBaseTileIndex[0]));
    bgb = MAP_create(&bgb_map, BG_B, TILE_ATTR_FULL(0, FALSE, FALSE, FALSE, bgBaseTileIndex[1]));

    MAP_scrollTo(bga, plx-32, ply-144);
    MAP_scrollTo(bgb, 640, 640);

    prepararBalas();
    prepararEnemigos();
    prepararMonedas();

    DMA_setBufferSize(9000);
    SYS_doVBlankProcess();
    DMA_setBufferSizeToDefault();

    MAP_scrollTo(bga, plx-32, ply-144);
    MAP_scrollTo(bgb, (plx-32)/2, (ply-144)/2);

    VDP_setWindowVPos(FALSE, 1);

    pldir = TRUE;
}

int main(u16 hard)
{
    JOY_init();
    JOY_setEventHandler( &prepararControles );

    XGM_setPCM(SFX_DISPARO, sfx_disparo, sizeof(sfx_disparo));
    XGM_setPCM(SFX_MATAR, sfx_matar, sizeof(sfx_matar));
    XGM_setPCM(SFX_SALTAR, sfx_saltar, sizeof(sfx_saltar));
    XGM_setPCM(SFX_MONEDA, sfx_moneda, sizeof(sfx_moneda));
    XGM_setPCM(SFX_MORIR, sfx_morir, sizeof(sfx_morir));

    tile_id[0]=bga_tileset.numTile;
    tile_id[1]=bgc_tileset.numTile;

    fondo_id[0]=background1.palette->data;
    fondo_id[1]=background3.palette->data;

    SPR_init();
    jugador = SPR_addSprite(&sprJugador,plx-8,ply,TILE_ATTR(PAL1,0, FALSE, FALSE));

    nivel_actual=0;
    cargarNivel();

    while (1)
    {
        moverJugador();
        checarBalasEnPantalla();
        checarEnemigosEnPantalla();

        revisarColisionBalasEnemigos();

        if (plx<160) { showx=0; scr_l=0; scr_r=320; }
        else if (plx>=ORILLA_DER-160) { showx=-160-160; scr_l=ORILLA_DER-320; scr_r=ORILLA_DER; }
        else { showx=plx-160; scr_l=plx-160; scr_r=plx+160; }

        if (ply<112) { showy=0; scr_u=0; scr_d=224; }
        else if (ply>=ORILLA_ABJ-112) { showy=-112-160-16; scr_u=ORILLA_ABJ-224; scr_d=ORILLA_ABJ; }
        else { showy=ply-112; scr_u=ply-112; scr_d=ply+112; }

        posicionBalas();
        posicionEnemigos();
        posicionMonedas();

        actualizarMonedas();

        desplazarPantalla();

        SPR_update();

        SYS_doVBlankProcess();
    }

    MEM_free(bga);
    MEM_free(bgb);

    return 0;
}
