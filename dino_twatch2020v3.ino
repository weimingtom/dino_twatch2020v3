#include "config.h"

//board: TTGO T-Watch
//PSRAM: Enabled
//board revision: T-Watch-2020-V3
//Partion: Default
//
//arduino ide 1.8.4
//esp32 2.0.2 for t-watch
//esp32 2.0.11 for t-display-s3

//see https://github.com/makerdiary/nrf52840-m2-devkit/blob/master/examples/python/dino/dino.py
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "dino_images.h"

TTGOClass *ttgo;

//----------------------------

enum Direction {
  INPUT_,
};

enum Pull {
  UP_,
};

class DigitalIO  
{
public:
  DigitalIO(int io): m_io(io){ value = 1; }
  virtual ~DigitalIO() {}
  static DigitalIO *DigitalInOut(int io){ 
    return new DigitalIO(io); 
  }
  enum Direction direction;
  enum Pull pull;
private:
  int m_io;
public:
  int value;
};

class Palette
{
public:
  Palette() : m_value0(0), m_value1(0) {}
  virtual ~Palette() {}
  void make_transparent(int index) {
    if (index == 0) {
      m_value0 = (0x00FFFFFF & m_value0);
    } else if (index == 1) {
      m_value1 = (0x00FFFFFF & m_value1);
    } else {
      m_value1 = (0x00FFFFFF & m_value1);
    }
  }
  void set(int index, unsigned int value) {
    if (index == 0) {
      m_value0 = (0xFF000000 | value);
    } else if (index == 1) {
      m_value1 = (0xFF000000 | value);
    } else { }
  }
  unsigned int get(int index) {
    if (index == 0) {
      return m_value0;
    } else if (index == 1) {
      return m_value1;
    } else {
      return 0;
    }
  }
private:
  unsigned int m_value0;
  unsigned int m_value1;
};

class Bitmap  
{
public:
  Bitmap(const char *filename, int bitmapType, int paletteType, Palette** pPalette) {
    this->pixels = loadBMPRaw(filename, &this->width, &this->height, 1, 0);
    this->palette = *pPalette = new Palette();
  }
  virtual ~Bitmap() {}
private:
  unsigned char * loadBMPRaw(const char * filename, 
    int* outWidth, int* outHeight, 
    int flipY, int flipToBGR)
  {
    if (strcmp(filename, "/img/cactus15x32.bmp") == 0) {
      *outWidth = cactus15x32_bmp_width;
      *outHeight = cactus15x32_bmp_height;
      return cactus15x32_bmp;
    } else if (strcmp(filename, "/img/cactus24x50.bmp") == 0) {
      *outWidth = cactus24x50_bmp_width;
      *outHeight = cactus24x50_bmp_height;
      return cactus24x50_bmp;
    } else if (strcmp(filename, "/img/game_over190x10.bmp") == 0) {
      *outWidth = game_over190x10_bmp_width;
      *outHeight = game_over190x10_bmp_height;
      return game_over190x10_bmp;
    } else if (strcmp(filename, "/img/cloud92x27.bmp") == 0) {
      *outWidth = cloud92x27_bmp_width;
      *outHeight = cloud92x27_bmp_height;
      return cloud92x27_bmp;
    } else if (strcmp(filename, "/img/digital200x25.bmp") == 0) {
      *outWidth = digital200x25_bmp_width;
      *outHeight = digital200x25_bmp_height;
      return digital200x25_bmp;
    } else if (strcmp(filename, "/img/ground512x12.bmp") == 0) {
      *outWidth = ground512x12_bmp_width;
      *outHeight = ground512x12_bmp_height;
      return ground512x12_bmp;
    } else if (strcmp(filename, "/img/dinosaur132x47.bmp") == 0) {
      *outWidth = dinosaur132x47_bmp_width;
      *outHeight = dinosaur132x47_bmp_height;
      return dinosaur132x47_bmp;
    }
    return 0;
  }
public: 
  int width;
  int height;
  Palette *palette;
  unsigned char *pixels;
};

class AdafruitImageload {
public:
  static Bitmap *load(const char *filename, int bitmapType, int paletteType, Palette** pPalette) {
    return new Bitmap(filename, bitmapType, paletteType, pPalette);
  }
};

class DisplayIO  
{
public:
  DisplayIO() {}
  virtual ~DisplayIO() {}
  enum {Bitmap_ = 1001, Palette_ = 1002, };
};

class TileGrid
{
public:
  TileGrid(Bitmap *color_bitmap, Palette *pixel_shader, 
    int width = 1, int height = 1, 
    int tile_width = 0, int tile_height = 0, 
    int x = 0, int y = 0, int default_tile = 0) {
    //init
    this->bmp = color_bitmap;
    this->pixel_shader = (pixel_shader != NULL) ? pixel_shader : color_bitmap->palette; //FIXME:???
    this->x = x;
    this->y = y;
    this->hidden = false;
  
    this->tileWidth = (tile_width > 0 && tile_width <= bmp->width) ? tile_width : bmp->width;
    this->tileHeight = (tile_height > 0 && tile_height <= bmp->height) ? tile_height : bmp->height;    
    this->numGridW = (width > 0) ? width : 1; 
    this->numGridH = (height > 0) ? height : 1;

    //this->m_defaultTile = (default_tile >= 0 && default_tile < this->m_tileIdCount) ? default_tile : 0;
    //this->m_tileIdCount = (bmp->width / this->tileWidth) * (bmp->height / this->tileHeight);
    this->m_defaultTile = default_tile;
    
    this->m_tileMap = new int[this->numGridW * this->numGridH];
    for (int k = 0; k < this->numGridW * this->numGridH; ++k) {
      this->m_tileMap[k] = this->m_defaultTile;
    }
  }
  virtual ~TileGrid() {}
  void set(int x, int value) { this->set2(x, 0, value); }
  void set2(int x, int y, int value) { this->m_tileMap[y * this->numGridW + x] = value; }
  int get2(int x, int y) { return this->m_tileMap[y * this->numGridW + x]; }
  void set_flip_x(bool value) { /*FIXME:TODO*/ }
private:
  int m_defaultTile;
  int *m_tileMap;
  //int m_tileIdCount;
public:
  Palette *pixel_shader;
  Bitmap *bmp;
  bool hidden;
  int tileWidth;
  int tileHeight;
  int numGridW; //==width
  int numGridH; //==height
  int x;
  int y;
};

extern void globalDisplayShowAll();
class Group
{
public:
  Group(int max_size) {
    this->m_max_size = (max_size > 0) ? max_size : 10;
    this->m_tile_grid = new TileGrid *[this->m_max_size];
    this->m_tile_grid_size = 0;
  }
  virtual ~Group() {}
  void append(TileGrid *tileGrid) {
      this->m_tile_grid[this->m_tile_grid_size++] = tileGrid;
      globalDisplayShowAll();
  }
  TileGrid *get(int index) {
    if (index >= 0 && index < this->m_tile_grid_size) {
      return this->m_tile_grid[index];
    } else {
      return NULL;
    }
  }
  int get_size() { return this->m_tile_grid_size;}
private:
  TileGrid **m_tile_grid;
  int m_tile_grid_size;
  int m_max_size;
};

class Display  
{
public:
  Display() {
    this->m_max_size = 30;
    this->m_group_list = new Group *[this->m_max_size];
    this->m_group_list_size = 0;
  }
  virtual ~Display() {}
  void show(Group *group) {
      this->m_group_list[this->m_group_list_size++] = group;
      if (this->m_group_list_size > 1) {
        this->m_group_list_size = 1;
      }
      globalDisplayShowAll();
  }
  Group *get(int index) {
    if (index >= 0 && index < this->m_group_list_size) {
      return this->m_group_list[index];
    } else {
      return NULL;
    }
  }
  int get_size() { return this->m_group_list_size;} 
  void clear() {
    this->m_group_list_size = 0;
    for (int i = 0; i < this->m_max_size; ++i) {
      this->m_group_list[i] = 0;
    }
  }
private:
  Group **m_group_list;
  int m_group_list_size;
  int m_max_size;  
};

class Board  
{
public:
  Board() {}
  virtual ~Board() {}
  enum { USR_BTN = 0,};
  static Display *DISPLAY_;
};

Display *Board::DISPLAY_ = new Display();

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
unsigned char *temp_buffer = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT * 2];
int refreshTime = 0;
void globalDisplayShowAll() {
  //refreshTime++;
  //if (refreshTime % 10 != 0) return; //skip frame;

  //ttgo->tft->fillScreen(TFT_BLACK);
  //ttgo->tft->startWrite();
  memset(temp_buffer, 0x00, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

  //Serial.println("globalDisplayShowAll ");
  for (int kk = 0; kk < Board::DISPLAY_->get_size(); ++kk) {
    //Serial.println("hellokk " + kk);
    Group *group = Board::DISPLAY_->get(kk);
    for (int k = 0; k < group->get_size(); ++k) {
      //delay(100);
      //Serial.println("hellok " + k);
      TileGrid *tile = group->get(k);
      if (tile != NULL) {
        bool hidden = tile->hidden;
        if (hidden) {
          continue;
        }
        Bitmap *bmp = tile->bmp;
        int x = tile->x;
        int y = tile->y;
        Palette *palette = tile->pixel_shader;
        unsigned char *pixels = bmp->pixels;
        //int width = bmp->width;
        //int height = bmp->height;
        int numGridW = tile->numGridW;
        int numGridH = tile->numGridH;
        int tileWidth = tile->tileWidth;
        int tileHeight = tile->tileHeight;
        int numTileW = bmp->width / tile->tileWidth;
        int numTileH = bmp->height / tile->tileHeight;
        //Serial.println("numGridH:" + String(numGridH) + ", numGridW:" + String(numGridH) + ", width:" + String(bmp->width) + ", height:" + String(bmp->height) + ", tileWidth:" + tileWidth);
        //ttgo->tft->fillRect(x, y, numGridW * tileWidth, numGridH * tileHeight, TFT_BLACK);
        for (int jj = 0; jj < numGridH; ++jj) {
          for (int ii = 0; ii < numGridW; ++ii) {
            int value = tile->get2(ii, jj); //which index of bmp section
            //value = 0;
            int valueH = value / numTileW;
            int valueW = value - valueH * numTileW;
            //ttgo->tft->fillRect(x + ii * tileWidth, y + jj * tileHeight, tileWidth, tileHeight, TFT_BLACK);
            for (int j = 0; j < tileHeight; ++j) {
              int jOffset = j + valueH * tileHeight;
              for (int i = 0; i < tileWidth; ++i) {
                int iOffset = i + valueW * tileWidth;
                int offTotal = jOffset * bmp->width + iOffset;
                int index_color = ((pixels[offTotal / 8]) & ((1 << (offTotal % 8)) & 0xff)) ? 1 : 0;
                unsigned int color = palette->get(index_color);
                if (color & 0xff000000) {
                  //color &= 0xffffff;
                  //ttgo->tft->writePixel(x + i + ii * tileWidth, y + j + jj * tileHeight, ttgo->tft->color24to16(color));
                  //ttgo->tft->drawPixel(x + i + ii * tileWidth, y + j + jj * tileHeight, ttgo->tft->color24to16(color)); //TFT_WHITE/*
                  if (x + i + ii * tileWidth < 240 && y + j + jj * tileHeight < 240) {
                    uint16_t color2 = ttgo->tft->color24to16(color);
                    temp_buffer[(y + j + jj * tileHeight) * 240 * 2 + (x + i + ii * tileWidth) * 2 + 0] = ((color2 & 0xff00) >> 8) & 0xff;
                    temp_buffer[(y + j + jj * tileHeight) * 240 * 2 + (x + i + ii * tileWidth) * 2 + 1] = ((color2 & 0xff)   >> 0) & 0xff;
                  }
                  //ttgo->tft->drawPixel(x + i + ii * tileWidth, y + j + jj * tileHeight, TFT_WHITE);
                } else {
                  //alpha color, skip
                  //ttgo->tft->drawPixel(x + i + ii * tileWidth, y + j + jj * tileHeight, TFT_WHITE);
                }
                //delay(3);
              }
            }
          }
        }
      }
    }
  }

  //ttgo->tft->endWrite();
  //https://blog.csdn.net/qq_41868901/article/details/105050371
  //ttgo->tft->drawBitmap(0, 0, temp_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_WHITE); // not good, only for bmp file ?
#if 0
  //not good
  ttgo->tft->startWrite();
  ttgo->tft->setAddrWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); /* set the working window */
  ttgo->tft->pushPixelsDMA(( uint16_t *)temp_buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
  ttgo->tft->endWrite();
#else
  ttgo->tft->setAddrWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); /* set the working window */
  ttgo->tft->pushColors(( uint16_t *)temp_buffer, SCREEN_WIDTH * SCREEN_HEIGHT, false);
#endif  
}

//---------------------------------------------










































//--------------------------


unsigned char os_urandom(int count, int index) 
{
  if (count == 1 && index == 0) {
    return random(0, 0xff); //return rand() & 0xff;
  } else {
    return 0;/*FIXME*/
  }
}

void time_sleep(double second) 
{
  /*FIXME:*/
  //delay((long)(second * 1000));
}

void update_score(TileGrid *score, int points) 
{
  printf("update_score: %d\n", points);
    score->set(2, points % 10);
    score->set(1, (points / 10) % 10);
    score->set(0, (points / 100) % 10);
}

int main__()
{
  Board::DISPLAY_->clear();
  
  DigitalIO *button = DigitalIO::DigitalInOut(Board::USR_BTN);
  button->direction = INPUT_;
  button->pull = UP_;

  Display *display = Board::DISPLAY_;

  Group *group = new Group(8);

  //Load bitmap
  Palette *palette = NULL;
  Palette *palette2 = NULL;
  Bitmap *ground_bmp = AdafruitImageload::load("/img/ground512x12.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette);

  palette->make_transparent(1);
  palette->set(0, 0x808000);

  TileGrid *ground = new TileGrid(ground_bmp, palette,
                            120,
                            1,
                            2,
                            12,
                            0,
                            192);

  for (int x = 0; x < 120; ++x) {
    ground->set(x, x);
  }

  group->append(ground);

  Bitmap *dinosaur_bmp = AdafruitImageload::load("/img/dinosaur132x47.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette);

  palette->make_transparent(1);
  palette->set(0, 0x404040);

  // 44x47
  TileGrid *dinosaur = new TileGrid(dinosaur_bmp, palette,
                  1,
                  1,
                  44,
                  47,
                  0,
                  0,
                  2);
  dinosaur->x = 8;
  dinosaur->y = 160;

  group->append(dinosaur);

#if 0
  display->show(group);
#else
  Bitmap *cactus_bmp = AdafruitImageload::load("/img/cactus15x32.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette);
  palette->make_transparent(1);
  palette->set(0, 0x008000);

  TileGrid *cactus = new TileGrid(cactus_bmp, palette);
  cactus->x = 240;
  cactus->y = 170;
  group->append(cactus);

  Bitmap *cactus2_bmp = AdafruitImageload::load("/img/cactus24x50.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette);
  palette->make_transparent(1);
  palette->set(0, 0x008000);

  TileGrid *cactus2 = new TileGrid(cactus2_bmp, palette);
  cactus2->set_flip_x(true);
  cactus2->y = 170;
  group->append(cactus2);

  Bitmap *digital_bmp = AdafruitImageload::load("/img/digital200x25.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette);
  palette->make_transparent(1);
  palette->set(0, 0x222222);

  TileGrid *score = new TileGrid(digital_bmp, palette,
                  3,
                  1,
                  20,
                  25);
  score->x = 176;
  score->y = 4;
  //score->hidden = true;
  group->append(score);

  Bitmap *game_over_bmp = AdafruitImageload::load("/img/game_over190x10.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette2);
  palette2->make_transparent(1);
  palette2->set(0, 0xFF0000);

  TileGrid *game_over = new TileGrid(game_over_bmp, palette2);
  game_over->x = 240;
  game_over->y = 100;
  game_over->hidden = true;
  group->append(game_over);

  Bitmap *cloud_bmp = AdafruitImageload::load("/img/cloud92x27.bmp",
    DisplayIO::Bitmap_,
    DisplayIO::Palette_, &palette2);
  palette2->make_transparent(15);

  TileGrid *cloud = new TileGrid(cloud_bmp, palette2);
  game_over->x = 80;
  game_over->y = 30;
  group->append(cloud);

  display->show(group);

  bool demo = true;

  while (true) {
    int t = 0;
    int v[] = {-32, -16, -8, -6, -5, -4, -3, -2, -2, -1, -1, -1, -1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 4, 5, 6, 8, 16, 32};
    int jump = -1;

    int points = 0;
    update_score(score, 0);

    dinosaur->y = 160;
    cactus->x = 240;
    cactus2->x = 480;

    while (true) {
      for(int x = 0; x < 120; ++x) {
        ground->set(x, (t + x) & 0xFF);
      }

      cactus->x = cactus->x - 2;
      if (cactus->x < -15) {
        cactus->x = cactus2->x + 240 + 2 * os_urandom(1, 0);
        printf("%d, %d\n", cactus->x, cactus2->x);
      }

      cactus2->x = cactus2->x - 2;
      if (cactus2->x < -24) {
        cactus2->x = cactus->x + 240 + 2 * os_urandom(1, 0);
        printf("%d, %d\n", cactus->x, cactus2->x);
      }   
    
      if ((t & 7) == 0) {
        cloud->x = cloud->x - 1;
        if (cloud->x < -92) {
          cloud->x = 240 + (os_urandom(1, 0) & 0xF);
        }
      }

      if (jump < 0 && button->value == 0) {
        jump = 0;
        dinosaur->set2(0, 0, 2);
        if (demo) {
          demo = false;
          points = 0;
          update_score(score, 0);
          palette->set(0, 0xCCCCCC);
          dinosaur->pixel_shader = palette;
        }
      }

      if (demo && jump < 0 && (cactus->x == 52 || cactus2->x == 52)) {
        jump = 0;
        dinosaur->set2(0, 0, 2);
      }

      if (jump >= 0) {
        dinosaur->y = dinosaur->y + v[jump];
        jump += 1;
        if (jump >= (sizeof(v) / sizeof(v[0]))) {
          jump = -1;
        }
      } else {
        if ((t & 7) == 0) {
          dinosaur->set2(0, 0, (t >> 3) & 1);
          //printf("dinosaur->set2 : %d\n",  (t >> 3) & 1);
        }
      }

      if (cactus->x == 0 || cactus2->x == 0) {
        points += 1;
        update_score(score, points);
      }

      display->show(group);
      //delay(100);

      if ((8 < cactus->x && cactus->x < 36 && (dinosaur->y + 44) >= cactus->y) || 
        (8 < cactus2->x && cactus2->x < 36 && (dinosaur->y + 44) >= cactus2->y)
        ) {
        printf("%d, %d, %d\n", cactus->x, cactus2->x, dinosaur->y);
        game_over->hidden = false;
        game_over->x = 25;
        display->show(group);
        time_sleep(1);
        while (button->value) {
          time_sleep(0.1);
        }

        time_sleep(1);
        while (!button->value) {
          time_sleep(0.1);
        }
        game_over->x = 240;
        game_over->hidden = true;
        printf("%s\n", "restart");
        break;
      }

      t = (t + 1) & 0xFF;
      // time_sleep(0.05)
    }
  }
#endif

  return 0;
}



void setup()
{
    Serial.begin(115200);
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
}

void loop()
{
    main__();
    Serial.println("R");
    //ttgo->tft->fillScreen(TFT_RED);
    delay(1000);
    Serial.println("G");
    //ttgo->tft->fillScreen(TFT_GREEN);
    delay(1000);
    Serial.println("B");
    //ttgo->tft->fillScreen(TFT_BLUE);
    delay(1000);
}



