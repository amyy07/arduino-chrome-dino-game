#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27,16,2);

const int upButton = 2;
const int downButton = 4;
const int selectButton = 5;
const int buzzer = 8;

enum GameState
{
    MAIN_MENU,
    CHARACTER_MENU,
    MAP_MENU,
    DIFFICULTY_MENU,
    HIGHSCORE_MENU,
    PLAYING,
    GAME_OVER
};

GameState state = MAIN_MENU;

// ---------------- Main Menu ----------------

enum CharacterType
{
    DINO,
    ROBOT,
    CAT,
    ALIEN,

    CHARACTER_COUNT
};

CharacterType currentCharacter = DINO;

const char* mainMenu[] =
{
    "Play",
    "Character",
    "Map",
    "Difficulty",
    "High score"
};

const char* characterNames[] =
{
    "Dino",
    "Robot",
    "Cat",
    "Alien"
};

const char* mapNames[] =
{
    "Desert",
    "Snow",
    "Forest",
    "Space"
};

const char* difficultyNames[] =
{
    "Easy",
    "Medium",
    "Hard"
};

enum MapType
{
    DESERT,
    SNOW,
    FOREST,
    SPACE,

    MAP_COUNT
};



enum Difficulty
{
    EASY,
    MEDIUM,
    HARD,

    DIFFICULTY_COUNT
};


MapType currentMap = DESERT;

Difficulty currentDifficulty = MEDIUM;

long score = 0;

int highScore = 0;

bool gameOver = false;

bool soundOn = true;

bool obstacleIsBird = false;

int obstaclePos = 16;

int reactionDistance = 15;

bool gameStarted = false;

int gameSpeed = 300;

unsigned long lastMove = 0;

int jumpTimer = 0;

int dinoRow = 1;
int menuIndex = 0;

const int ADDR_HIGHSCORE = 0;

const int ADDR_CHARACTER = 1;

const int ADDR_MAP = 2;

const int ADDR_DIFFICULTY = 3;

const int ADDR_SOUND = 4;

int characterIndex = 0;

int mapIndex = 0;

int difficultyIndex = 0;

bool menuNeedsRedraw = true;

unsigned long lastAnimation = 0;

int speedIncrease = 15;

bool newRecord = false;

int birdChance = 30;

bool upReleased = true;

const int MENU_ITEMS = 5;

// ---------------- Characters ----------------

byte dino[8] =
{
B00110,
B00111,
B00110,
B01110,
B11110,
B01100,
B01010,
B10001
};

byte robot[8] =
{
 B01110,
  B10101,
  B01110,
  B00100,
  B11111,
  B00100,
  B01110,
  B01110
};

byte cat[8] =
{
B00100,
  B00111,
  B00110,
  B00111,
  B00010,
  B11110,
  B01110,
  B01010
};

byte alien[8] =
{
B01100,
  B10110,
  B10101,
  B00111,
  B00100,
  B01110,
  B00110,
  B01111
};

bool runFrame = false;

byte dinoRun[8] =
{
B00110,
B00111,
B00110,
B01110,
B11110,
B00110,
B01100,
B10010
};

byte robotRun[8] =
{
B01110,
  B10101,
  B01110,
  B00100,
  B11111,
  B00100,
  B11111,
  B10001
};

byte catRun[8] =
{
B00100,
  B00111,
  B00110,
  B00111,
  B00010,
  B11110,
  B01111,
  B10010
};

byte alienRun[8] =
{
B01100,
  B10110,
  B10101,
  B00111,
  B00100,
  B01110,
  B00110,
  B00110
};

//------- map props

byte cactus[8]={
B00000,
B00100,
B00101,
B10101,
B10110,
B01100,
B00100,
B00100
};

byte snowman[8] =
{
B00100,
B01110,
B01110,
B00100,
B01110,
B11111,
B01110,
B00100
};

byte tree[8] =
{
B00100,
B01110,
B11111,
B00100,
B01110,
B11111,
B00100,
B00100
};

byte asteroid[8] =
{
B01100,
B11110,
B11111,
B01110,
B11111,
B11110,
B01100,
B00000
};

byte bird[8] =
{
B00000,
B01010,
B11111,
B01110,
B00100,
B01010,
B00000,
B00000
};

byte icicle[8] =
{
B11110,
  B01100,
  B11100,
  B01100,
  B00110,
  B00110,
  B00100,
  B01000
};

byte owl[8] = 
{
  B10001,
  B01110,
  B10001,
  B10101,
  B01110,
  B11111,
  B11110,
  B01100
};


byte ufo[8] =
{
B00000,
  B00100,
  B01110,
  B11011,
  B01110,
  B00100,
  B01010,
  B10001
};

void setup()
{
    pinMode(upButton, INPUT_PULLUP);
    pinMode(downButton, INPUT_PULLUP);
    pinMode(selectButton, INPUT_PULLUP);
    pinMode(buzzer,OUTPUT);

    lcd.init();
    lcd.backlight();

    lcd.createChar(0,dino);
    lcd.createChar(1,robot);
    lcd.createChar(2,cat);
    lcd.createChar(3,alien);

    randomSeed(analogRead(A2));

    highScore = EEPROM.read(ADDR_HIGHSCORE);

    if(highScore==255)
    {
        highScore=0;
        EEPROM.write(ADDR_HIGHSCORE,0);
    }

    currentCharacter =
    (CharacterType)EEPROM.read(ADDR_CHARACTER);

    if(currentCharacter >= CHARACTER_COUNT)
    {
    currentCharacter = DINO;
    EEPROM.write(ADDR_CHARACTER,currentCharacter);
    }

    characterIndex = currentCharacter;

    currentMap = (MapType)EEPROM.read(ADDR_MAP);

    if(currentMap >= MAP_COUNT)
    {
    currentMap = DESERT;
    EEPROM.write(ADDR_MAP,currentMap);
    }

    mapIndex = currentMap;

    currentDifficulty = (Difficulty)EEPROM.read(ADDR_DIFFICULTY);

    if(currentDifficulty >= DIFFICULTY_COUNT)
    {
        currentDifficulty = MEDIUM;
        EEPROM.write(ADDR_DIFFICULTY,currentDifficulty);
    }

    difficultyIndex = currentDifficulty;
    }

void startGame()
{
    lcd.clear();

    switch(currentMap)
    {
        case DESERT:
            lcd.print("Desert");
            break;

        case SNOW:
            lcd.print("Snow");
            break;

        case FOREST:
            lcd.print("Forest");
            break;

        case SPACE:
            lcd.print("Space");
            break;
    }
    delay(1000);
    lcd.clear();

    switch(currentDifficulty)
    {
        case EASY:
            lcd.print("Easy");
            break;

        case MEDIUM:
            lcd.print("Medium");
            break;

        case HARD:
            lcd.print("Hard");
            break;
    }

    delay(700);
    lcd.clear();
    
    score = 0;

    gameOver = false;

    dinoRow = 1;

    jumpTimer = 0;

    runFrame = false;

    applyDifficulty();

    obstaclePos = reactionDistance+random(3,8);

    obstacleIsBird = random(100) < 30;

    lastMove = millis();

    gameStarted = true;

    newRecord = false;

    upReleased = true;

    lcd.clear();

    loadSprites();

    lcd.setCursor(6,0);
    lcd.print("3");
    tone(buzzer,1000,100);
    delay(500);

    lcd.clear();
    lcd.setCursor(6,0);
    lcd.print("2");
    tone(buzzer,1000,100);
    delay(500);

    lcd.clear();
    lcd.setCursor(6,0);
    lcd.print("1");
    tone(buzzer,1000,100);
    delay(500);

    lcd.clear();
}

void loop()
{
    switch(state)
    {
        case MAIN_MENU:
            mainMenuLoop();
            break;

        case CHARACTER_MENU:
            characterMenuLoop();
            break;

        case MAP_MENU:
            mapMenuLoop();
            break;

        case DIFFICULTY_MENU:
            difficultyMenuLoop();
            break;

        case HIGHSCORE_MENU:
            highScoreLoop();
            break;

        case PLAYING:
            gameLoop();
            break;

        case GAME_OVER:
            gameOverLoop();
            break;
    }
}

void readButtons()
{
    static bool upReleased = true;
    static bool downReleased = true;

    if(digitalRead(upButton) == LOW && upReleased)
    {
        menuIndex--;

        if(menuIndex < 0)
            menuIndex = MENU_ITEMS - 1;

        tone(buzzer, 1800, 20);

        menuNeedsRedraw = true;
        upReleased = false;
    }

    if(digitalRead(upButton) == HIGH)
        upReleased = true;

    if(digitalRead(downButton) == LOW && downReleased)
    {
        menuIndex++;

        if(menuIndex >= MENU_ITEMS)
            menuIndex = 0;

        tone(buzzer, 1800, 20);

        menuNeedsRedraw = true;
        downReleased = false;
    }

    if(digitalRead(downButton) == HIGH)
        downReleased = true;
}

void drawMainMenu()
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("> ");
    lcd.print(mainMenu[menuIndex]);

    int secondLine = menuIndex + 1;

    if(secondLine >= MENU_ITEMS)
        secondLine = 0;

    lcd.setCursor(2,1);
    lcd.print(mainMenu[secondLine]);
}

void mainMenuLoop()
{
    readButtons();

    if(menuNeedsRedraw)
    {
        drawMainMenu();
        menuNeedsRedraw = false;
    }

    if(digitalRead(selectButton) == LOW)
    {
        tone(buzzer,1200,40);

        delay(200);

        switch(menuIndex)
        {
            case 0:

            startGame();
            state = PLAYING;
            break;

            case 1:
                menuNeedsRedraw = true;
                state = CHARACTER_MENU;
                break;

            case 2:
                menuNeedsRedraw = true;
                state = MAP_MENU;
                break;

            case 3:
                menuNeedsRedraw = true;
                state = DIFFICULTY_MENU;
                break;

            case 4:
                menuNeedsRedraw = true;
                state = HIGHSCORE_MENU;
                break;
        }
    }
}

void drawCharacterMenu()
{
    lcd.setCursor(0,0);
    lcd.print("                ");  

    lcd.setCursor(0,1);
    lcd.print("                ");   

    lcd.setCursor(0,0);
    lcd.print("> ");
    lcd.print(characterNames[characterIndex]);

    int next = characterIndex + 1;

    if(next >= CHARACTER_COUNT)
        next = 0;

    lcd.setCursor(2,1);
    lcd.print(characterNames[next]);
}

void characterMenuLoop()
{
    static bool upReleased = true;
        static bool downReleased = true;

        if(digitalRead(upButton) == LOW && upReleased)
        {
            characterIndex--;

            tone(buzzer, 1800, 20);

            if(characterIndex < 0)
                characterIndex = CHARACTER_COUNT - 1;

            upReleased = false;

            menuNeedsRedraw = true;
        }

        if(digitalRead(upButton) == HIGH)
            upReleased = true;

        if(digitalRead(downButton) == LOW && downReleased)
        {
            characterIndex++;

            tone(buzzer, 1800, 20);

            if(characterIndex >= CHARACTER_COUNT)
                characterIndex = 0;

            downReleased = false;

            menuNeedsRedraw = true;
        }

        if(digitalRead(downButton) == HIGH)
            downReleased = true;

            if(menuNeedsRedraw)
        {
            drawCharacterMenu();
            menuNeedsRedraw = false;
        }

    if(digitalRead(selectButton)==LOW)
    {
        currentCharacter =
        (CharacterType)characterIndex;

        EEPROM.write(
            ADDR_CHARACTER,
            currentCharacter
        );

        tone(buzzer,1200,50);

        delay(250);

        menuNeedsRedraw = true;
        state = MAIN_MENU;
    }
}

void drawMapMenu()
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("> ");
    lcd.print(mapNames[mapIndex]);

    int next = mapIndex + 1;

    if(next >= MAP_COUNT)
        next = 0;

    lcd.setCursor(2,1);
    lcd.print(mapNames[next]);
}


void mapMenuLoop()
{
        static bool upReleased = true;
        static bool downReleased = true;

        if(digitalRead(upButton) == LOW && upReleased)
        {
            mapIndex--;

            tone(buzzer, 1800, 20);

            if(mapIndex < 0)
               mapIndex = MAP_COUNT - 1;

            upReleased = false;

            menuNeedsRedraw = true;
        }

        if(digitalRead(upButton) == HIGH)
            upReleased = true;

        if(digitalRead(downButton) == LOW && downReleased)
        {
            mapIndex++;

            tone(buzzer, 1800, 20);

            if(mapIndex >= MAP_COUNT)
                mapIndex = 0;

            downReleased = false;

            menuNeedsRedraw = true;
        }

        if(digitalRead(downButton) == HIGH)
            downReleased = true;

        if(menuNeedsRedraw)
    {
        drawMapMenu();
        menuNeedsRedraw = false;
    }
    

    if(digitalRead(selectButton)==LOW)
    {
        currentMap = (MapType)mapIndex;

        EEPROM.write(
            ADDR_MAP,
            currentMap
        );

        tone(buzzer,1200,50);

        delay(250);

        menuNeedsRedraw = true;
        state = MAIN_MENU;
    }
}

void drawDifficultyMenu()
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("> ");
    lcd.print(difficultyNames[difficultyIndex]);

    int next = difficultyIndex + 1;

    if(next >= DIFFICULTY_COUNT)
        next = 0;

    lcd.setCursor(2,1);
    lcd.print(difficultyNames[next]);
}

void difficultyMenuLoop()
{
    static bool upReleased = true;
static bool downReleased = true;

if(digitalRead(upButton) == LOW && upReleased)
{
    difficultyIndex--;

    tone(buzzer, 1800, 20);

    if(difficultyIndex < 0)
        difficultyIndex = DIFFICULTY_COUNT - 1;

    upReleased = false;

    menuNeedsRedraw = true;
}

if(digitalRead(upButton) == HIGH)
    upReleased = true;

if(digitalRead(downButton) == LOW && downReleased)
{
    difficultyIndex++;

    tone(buzzer, 1800, 20);

    if(difficultyIndex >= DIFFICULTY_COUNT)
       difficultyIndex= 0;

    downReleased = false;

    menuNeedsRedraw = true;
}

if(digitalRead(downButton) == HIGH)
    downReleased = true;

    if(menuNeedsRedraw)
    {
        drawDifficultyMenu();
        menuNeedsRedraw = false;
    }

    if(digitalRead(selectButton) == LOW)
    {
        currentDifficulty = (Difficulty)difficultyIndex;

        EEPROM.write(ADDR_DIFFICULTY,currentDifficulty);

        tone(buzzer,1200,50);

        delay(250);

        menuNeedsRedraw = true;
        state = MAIN_MENU;
    }
}

void applyDifficulty()
{
    switch(currentDifficulty)
    {
        case EASY:
            gameSpeed = 350;
            reactionDistance = 16;
            speedIncrease = 10;
            birdChance = 20;
            break;

        case MEDIUM:
            gameSpeed = 280;
            reactionDistance = 14;
            speedIncrease = 15;
            birdChance = 30;
            break;

        case HARD:
            gameSpeed = 220;
            reactionDistance = 11;
            speedIncrease = 20;
            birdChance = 40;
            break;
    }
}

void loadSprites()
{
    switch(currentCharacter)
    {
        case DINO:
            lcd.createChar(0,dino);
            lcd.createChar(1,dinoRun);
            break;

        case ROBOT:
            lcd.createChar(0,robot);
            lcd.createChar(1,robotRun);
            break;

        case CAT:
            lcd.createChar(0,cat);
            lcd.createChar(1,catRun);
            break;

        case ALIEN:
            lcd.createChar(0,alien);
            lcd.createChar(1,alienRun);
            break;
    }

    switch(currentMap)
{
    case DESERT:
        lcd.createChar(2,cactus);
        lcd.createChar(3,bird);
        break;

    case SNOW:
        lcd.createChar(2,snowman);
        lcd.createChar(3,icicle);
        break;

    case FOREST:
        lcd.createChar(2,tree);
        lcd.createChar(3,owl);
        break;

    case SPACE:
        lcd.createChar(2,asteroid);
        lcd.createChar(3,ufo);
        break;
}

}

void highScoreLoop()
{
    if(menuNeedsRedraw)
    {
        lcd.setCursor(0,0);
        lcd.print("                ");

        lcd.setCursor(0,1);
        lcd.print("                ");

        lcd.setCursor(0,0);
        lcd.print("High Score");

        lcd.setCursor(0,1);
        lcd.print(highScore);

        menuNeedsRedraw = false;
    }

    if(digitalRead(selectButton) == LOW)
    {
        tone(buzzer,1200,50);
        delay(200);

        menuNeedsRedraw = true;
        state = MAIN_MENU;
    }
}

void gameLoop()
{
    // ---------- Jump ----------
    if(digitalRead(upButton) == LOW && upReleased && jumpTimer == 0)
    {
        tone(buzzer, 1400, 40);

        dinoRow = 0;
        jumpTimer = 7;

        upReleased = false;
    }

    if(digitalRead(upButton) == HIGH)
    {
        upReleased = true;
    }

    // ---------- Move obstacle ----------
    if(millis() - lastMove > gameSpeed)
    {
        lastMove = millis();

        if(jumpTimer > 0)
        {
            jumpTimer--;

            if(jumpTimer==0)
                dinoRow=1;
        }

        obstaclePos--;

        // Spawn next obstacle
        if(obstaclePos < 0)
        {
            obstaclePos = reactionDistance + random(0,3);

            obstacleIsBird = random(100) < birdChance;

            score++;

            if(score > highScore && !newRecord)
            {
                newRecord = true;

                highScore = score;

                EEPROM.write(ADDR_HIGHSCORE, highScore);

                tone(buzzer,900,80);
                delay(100);
                tone(buzzer,1200,80);
                delay(100);
                tone(buzzer,1600,120);
            }

            if(gameSpeed > 120)
            gameSpeed -= speedIncrease;

            if(gameSpeed < 90)
               gameSpeed = 90;
        }

        // ---------- Collision ----------
        if(obstaclePos == 1)
        {
            if(!obstacleIsBird && dinoRow == 1)
            {
                state = GAME_OVER;
                tone(buzzer,500,100);

                delay(120);

                tone(buzzer,250,250);
            }

            if(obstacleIsBird && dinoRow == 0)
            {
                state = GAME_OVER;
                tone(buzzer,500,100);

                delay(120);

                tone(buzzer,250,250);
            }
        }

        drawGame();

        if(digitalRead(selectButton)==LOW)
        {
            lcd.clear();
            lcd.print("Paused");

            while(digitalRead(selectButton)==LOW);

            delay(250);

            while(digitalRead(selectButton)==HIGH);

            delay(250);

            lcd.clear();
        }
    }
}

void drawGame()
{
    lcd.setCursor(0,0);
    lcd.print("                ");   // Clear top row

    lcd.setCursor(0,1);
    lcd.print("                ");   // Clear bottom row

    // Score
    lcd.setCursor(12,0);
    lcd.print(score);

    // Character
    lcd.setCursor(1,dinoRow);

    if(runFrame)
    {
        lcd.write(byte(1));   // Running frame
    }
    else
    {
        lcd.write(byte(0));   // Standing frame
    }

        if(millis()-lastAnimation>150)
    {
        runFrame=!runFrame;
        lastAnimation=millis();
    }

    // Obstacle
    if(obstacleIsBird)
    {
        lcd.setCursor(obstaclePos,0);
        lcd.write(byte(3));
    }
    else
    {
        lcd.setCursor(obstaclePos,1);
        lcd.write(byte(2));
    }
}

void gameOverLoop()
{

    if(score > highScore)
    {
        lcd.clear();

        lcd.print("NEW RECORD!");

        lcd.setCursor(0,1);
        lcd.print(score);

        tone(buzzer,1000,100);
        delay(100);
        tone(buzzer,1400,100);
        delay(100);
        tone(buzzer,1800,150);

        delay(1500);
    }

    lcd.clear();

    lcd.setCursor(3,0);
    lcd.print("GAME OVER");

    lcd.setCursor(0,1);
    lcd.print("S:");
    lcd.print(score);

    lcd.print(" H:");
    lcd.print(highScore);

    while(digitalRead(selectButton) == HIGH);

    delay(250);

    menuNeedsRedraw = true;
    state = MAIN_MENU;
}



