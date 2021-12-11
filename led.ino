//=====================    SETTINGS    =====================\\

#define STRIPS 1
#define bMAX 128
#define bMIN 2

#define IR 8
#define RED 9
#define GREEN 10
#define BLUE 6
//#define RED2 9
//#define GREEN2 10
//#define BLUE2 6


//====================    STRUCTURES    ====================\\

#include <IRremote.h>
IRrecv irrecv(IR);
decode_results result;

struct Color {
  int r;
  int g;
  int b;
};
bool operator==(const Color& lhs, const Color& rhs) {
  if (lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b)
    return true;
  return false; 
}
bool operator !=(const Color& lhs, const Color& rhs) {
  return !operator==(lhs,rhs);
}

class LedStrip {
  int _pinR;
  int _pinG;
  int _pinB;
  
  int _bright;
  int _lastClicked;
  int _timer;
  Color _current;
  
  bool _strobeUp;
  Color _fade;
  int _fadeIncrementing; //0 if green is incrementing, 1 if it's blue, 2 if red
  Color _custom;
  
  
 public:
  LedStrip() {};
  LedStrip(int pinR, int pinG, int pinB, Color custom = Color{255,67,0}) 
   : _pinR{pinR}, 
     _pinG{pinG},
     _pinB{pinB},
     _bright{bMAX},
     _lastClicked{23},
     _strobeUp{false},
     _timer{0},
     _current{Color{0,0,255}},
     _fade{Color{255,0,0}},
     _fadeIncrementing{1},
     _custom{custom}  {
    show();
  }
  
  int bright() { return _bright; }
  void moreBright() {
    _bright*=bMIN;
    if (_bright > bMAX) _bright = bMAX;
    show();
  }
  void lessBright() {
    _bright /= bMIN;
    if (_bright < bMIN) _bright = bMIN;
    show();
  }

  void setLastClicked(int n) { _lastClicked = n; }
  
  void show() {
    analogWrite(_pinR, _current.r *_bright/bMAX);
    analogWrite(_pinG, _current.g *_bright/bMAX);
    analogWrite(_pinB, _current.b *_bright/bMAX);
    _lastClicked = 10;
  }
  void setColor(int r, int g, int b) {
    _current.r = r;
    _current.g = g;
    _current.b = b;
    show();
  }
  void setColor(Color c) {
    _current = c;
    show();
  }

  void flash(int r, int g, int b, int n = 1) {
    for (int i = 0; i < n; i++) {
      delay(500);
      digitalWrite(_pinR,LOW);
      digitalWrite(_pinG,LOW);
      digitalWrite(_pinB,LOW);
      delay(500);
      analogWrite(RED, r);
      analogWrite(GREEN, g);
      analogWrite(BLUE, b);
    }
    delay(500);
  }
  void flash(Color set, int n) {
    flash(set.r, set.g, set.b, n);
  }

  void showColor(int r, int g, int b) {
    analogWrite(_pinR, r *_bright/bMAX);
    analogWrite(_pinG, g *_bright/bMAX);
    analogWrite(_pinB, b *_bright/bMAX);
  }
  void showColor(Color set) {
    showColor(set.r, set.g, set.b);
  }

  void turnOff()  {
    analogWrite(_pinR, LOW);
    analogWrite(_pinG, LOW);
    analogWrite(_pinB, LOW);
    if (_lastClicked < 100)
      _lastClicked += 100;
  }

  void turnOn() {
    show();
    _lastClicked-=100;
  }
  
  void next() {
    if (_lastClicked == 21) {                   //FLASH
      if (_timer < 6) {
        _timer++;
        return;
      }
      _timer = 0;
      int off = rand() % 3;
      if (off == 0)       showColor(0, rand()%255, rand()%255);
      else if (off == 1)  showColor(rand()%255, 0, rand()%255);
      else                showColor(rand()%255, rand()%255, 0);
    }
    
    if (_lastClicked == 23) {                    //FADE
      if (_timer < 1) {
        _timer++;
        return;
      }
      _timer = 0;
      
      if (_fadeIncrementing == 0) {
        _fade.b--;
        _fade.r++;
        if (_fade.b == 0) _fadeIncrementing = 1;
      }
      if (_fadeIncrementing == 1) {
        _fade.r--;
        _fade.g++;
        if (_fade.r == 0) _fadeIncrementing = 2;
      }
      if (_fadeIncrementing == 2) {
        _fade.g--;
        _fade.b++;
        if (_fade.g == 0) _fadeIncrementing = 0;
      }
      
      showColor(_fade);
    }

    if (_lastClicked == 24) {                   //SMOOTH (choose color)
      if (_current != _custom) {
        setColor(_custom);
        return;
      }             //else: choose a color

      flash(_custom, 2);
      
      int sel = 0;
      flash(255,0,0);
      
      //showColor(_custom);
    
      while (sel != -1) {
        if (irrecv.decode(&result)) {
          switch (result.value) {
           case 16748655:                 //SMOOTH  -MORE
            _lastClicked = -2;
            if (sel == 0) { _custom.r++;
              if (_custom.r > 255) _custom.r = 255;
            } 
            else if (sel == 1){ _custom.g++;
              if (_custom.g > 255) _custom.g = 255;
            }
            else if (sel == 2) { _custom.b++;
              if (_custom.b > 255) _custom.b = 255;
            }
            else if (sel == 3) {
              if (_custom.r*2 < 256 && _custom.g*2 < 256 && _custom.b*2 < 256) {
                _custom.r*=2;
                _custom.g*=2;
                _custom.b*=2;
              }
            }
            break;
           case 16758855:                 //SMOOTH  -LESS
            _lastClicked = -3;
            if (sel == 0) { _custom.r--;
              if (_custom.r < 0) _custom.r = 0;
            } 
            else if (sel == 1){ _custom.g--;
              if (_custom.g < 0) _custom.g = 0;
            }
            else if (sel == 2) { _custom.b--;
              if (_custom.b < 0) _custom.b = 0;
            }
            else if (sel == 3) {
              if ((_custom.r/2 > 1 || _custom.r == 0) && (_custom.g/2 > 1 || _custom.g == 0) && (_custom.b/2 > 1 || _custom.b == 0)) {
                _custom.r/=2;
                _custom.g/=2;
                _custom.b/=2;
              }
            }
            break;
           case 4294967295:               //SMOOTH  -LONG PRESSED
            if (_lastClicked == -2) {
              if (sel == 0) { _custom.r*=2;
                if (_custom.r > 255) _custom.r = 255;
              } 
              else if (sel == 1){ _custom.g*=2;
                if (_custom.g > 255) _custom.g = 255;
              }
              else if (sel == 2) { _custom.b*=2;
                if (_custom.b > 255) _custom.b = 255;
              }
            }
            else if (_lastClicked == -3) {
              if (sel == 0) { _custom.r/=2;
                //if (_custom.r < 0) _custom.r = 0;
              } 
              else if (sel == 1){ _custom.g/=2;
                //if (_custom.g < 0) _custom.g = 0;
              }
              else if (sel == 2) { _custom.b/=2;
                //if (_custom.b < 0) _custom.b = 0;
              }
            }
            break;
           case 16750695:                 //SMOOTH  -RED
            _lastClicked =  24;
            sel = 0;
            flash(255,0,0);
            break;
           case 16767015:                 //SMOOOTH  -GREEN
            _lastClicked =  24;
            sel = 1;
            flash(0,255,0);
            break;
           case 16746615:                 //SMOOTH  -BLUE
            _lastClicked =  24;
            sel = 2;
            flash(0,0,255);
            break;
           case 16754775:                 //SMOOTH  -WHITE
            _lastClicked =  24;
            sel = 3;
            flash(255,255,255);
            break;            
           case 16724175:                 //SMOOTH  -SMOOTH
            _lastClicked =  24;
            sel = -1;
            break;
         
          } //end switch  
          
          Serial.print("r: ");
          Serial.print(_custom.r);
          Serial.print("  -  g: ");
          Serial.print(_custom.g);
          Serial.print("  -  b: ");
          Serial.println(_custom.b);
          
          showColor(_custom);
          
          irrecv.resume();
        } //end if ir
        delay(100);
      } //end while
      
      flash(_custom, 3);
      setColor(_custom);
    
    } //end SMOOTH (lastClicked == 24)
  } //end next()
}; //end Class

int sS; //selected strip
LedStrip* strips;

  

//======================    SETUP.    ======================\\

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  sS = 0;
  LedStrip bottom(RED, GREEN, BLUE, Color{208,32,0});
  //LedStrip top(RED2, GREEN2, BLUE2, Color{36,0,134});
  strips = new LedStrip[STRIPS];
  strips[0] = bottom;
  //strips[1] = top;
}



//=======================    LOOP    =======================\\

void loop() {
  if (irrecv.decode(&result)) {                                 //have we received an IR signal?
    switch (result.value) {                                     //react to IR signal
     case 16748655: strips[sS].moreBright(); break;   //MORE
     case 16758855: strips[sS].lessBright(); break;   //LESS
     
     case 16775175:                             //OFF
     case 551519865:
      for(int i = 0; i < STRIPS; i++) {
        strips[i].turnOff();
      }
      break;
     case 16756815: 
     case 551505585:                            //ON
      for(int i = 0; i < STRIPS; i++) {
        strips[i].turnOn();
      }
      break;
      
     case 16750695: strips[sS].setColor(255,0,0);      break;   //RED
     case 16767015: strips[sS].setColor(0,255,0);      break;   //GREEN
     case 16746615: strips[sS].setColor(0,0,255);      break;   //BLUE
     case 16754775: strips[sS].setColor(255,255,255);  break;   //WHITE
     
     case 16771095: strips[sS].setColor(204,56,0);     break;   //COLOR 1
     case 16730295: strips[sS].setColor(153,250,153);  break;   //COLOR 2
     case 16738455: strips[sS].setColor(0,128,255);    break;   //COLOR 3
     case 16712445: strips[sS].setColor(255,140,0);    break;   //COLOR 4
     case 16724685: strips[sS].setColor(5,237,255);    break;   //COLOR 5
     case 16720095: strips[sS].setColor(153,51,204);   break;   //COLOR 6
     case 16732335: strips[sS].setColor(255,176,15);   break;   //COLOR 7
     case 16742535: strips[sS].setColor(0,230,237);    break;   //COLOR 8
     case 16740495: strips[sS].setColor(148,0,212);    break;   //COLOR 9
     case 16726215: strips[sS].setColor(255,255,0);    break;   //COLOR 10
     case 16722135: strips[sS].setColor(54,219,201);   break;   //COLOR 11
     case 16773135: strips[sS].setColor(255,51,179);   break;   //COLOR 12
     
     case 16757325: strips[sS].setLastClicked(21); break; //FLASH
     case 16711935:                                       //STROBE
      if (sS < STRIPS-1) sS++;
      else sS = 0;
      break;
     case 16734375: strips[sS].setLastClicked(23); break; //FADE
     case 16724175: strips[sS].setLastClicked(24); break; //SMOOTH
     
     default: Serial.println(result.value);
    }
    irrecv.resume();
  }
  
  for (int i = 0; i < STRIPS; i++) {      //Update strips
    strips[i].next();
  }
  delay(10);
}
