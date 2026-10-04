//
//  Config.hpp  --  every tunable constant for "No Proxy Today" lives here.
//
//  Include this AFTER "iGraphics.h" from iMain.cpp. Nothing else includes it.
//
#ifndef CONFIG_HPP
#define CONFIG_HPP

#pragma warning(disable:4996)   // fopen / sprintf are fine for this project

// ---------------------------------------------------------------------------
//  Window
// ---------------------------------------------------------------------------
// 1200x675 is 16:9. game_poster.jpeg is 1377x768 (1.793), so the poster fills
// the window with less than 1% vertical stretch -- effectively a perfect fit.
const int WIN_W = 1200;
const int WIN_H = 675;
const char WIN_TITLE[] = "No Proxy Today  -  A Dhaka Commute Adventure";

// ---------------------------------------------------------------------------
//  Colours  (0-255)
// ---------------------------------------------------------------------------
struct Color { int r, g, b; };

const Color C_ACCENT     = { 255, 170,  40 };   // logo orange
const Color C_ACCENT_DIM = { 190, 120,  20 };
const Color C_INK        = {  10,  16,  34 };   // near-black navy
const Color C_PANEL      = {   8,  14,  32 };
const Color C_TEXT       = { 240, 244, 255 };
const Color C_TEXT_DIM   = { 160, 174, 200 };
const Color C_SKY        = { 120, 200, 255 };   // pale blue, used for headings
const Color C_WHITE      = { 255, 255, 255 };

// ---------------------------------------------------------------------------
//  Main-menu layout
//
//  The poster's "NO PROXY TODAY" logo occupies roughly y = 262..521 on the
//  left of the window, so the button stack sits underneath it, in the free
//  strip between y = 8 and y = 244.
// ---------------------------------------------------------------------------
const double MENU_PANEL_X = 80.0;
const double MENU_PANEL_Y = 6.0;
const double MENU_PANEL_W = 352.0;
const double MENU_PANEL_H = 238.0;

// Five buttons now have to live in the same strip under the logo, so they are
// a little shorter and a little closer together: 5 x 40 + 4 x 7 = 228, which
// fits between y 10 and y 238 with the plate at 6..244. The logo starts at 262
// and is still clear of them.
const double BTN_X     = 96.0;
const double BTN_W     = 320.0;
const double BTN_H     = 40.0;
const double BTN_GAP   =  7.0;
const double BTN_TOP_Y = 238.0;   // y of the TOP edge of the first button

const int MENU_ITEM_COUNT = 5;

// ---------------------------------------------------------------------------
//  Sub-screen (Keys / About) layout
// ---------------------------------------------------------------------------
const double PAGE_MARGIN_X = 80.0;
const double PAGE_LINE_H   = 17.0;

// ---------------------------------------------------------------------------
//  Asset paths, relative to whichever root Paths.hpp discovers.
//
//  These are the EXACT names on disk, typos and all. Do not "correct" them:
//    game_assetes         the folder really is spelled that way
//    bike_is_comming.jpg  double m
//    motocycle horn.wav   missing r
//    Flying_birds.png     capital F
//    powerup_shield.png   spelled shield, not sheild
// ---------------------------------------------------------------------------
const char PATH_POSTER[]   = "game_poster.jpeg";

// The campaign map behind the level-select screen. It sits in the solution
// root beside game_poster.jpeg, so it resolves through the same assetPath().
const char PATH_LEVELMAP[] = "levelmap.jpeg";

// The ornate banner the HUD panels are drawn on. It is one long bar, so it
// is loaded in three pieces -- two end caps and a stretchable middle -- and
// reassembled at whatever width a panel happens to need. The numbers below
// are measured off the file: the bar occupies rows 20..68, its content runs
// from column 13 to 508, and the ornate ends are about 22 px wide.
//
// The central crest and the "BOSS NAME" tab are deliberately OUTSIDE those
// rows: five panels each carrying a crest and the words BOSS NAME would be
// nonsense, so only the bar itself is used.
const char PATH_HUD_FRAME[] = "hud_frame.png";
const int  HUD_FRAME_DARK_CUT = 26;   // the field it sits on is pure black
const int  HUD_FRAME_Y      = 20;
const int  HUD_FRAME_H      = 47;
const int  HUD_FRAME_CAP_W  = 22;
const int  HUD_FRAME_L_X    = 13;
// The middle is taken from a CLEAN stretch of the bar, x 60..190. The crest
// attaches at x 196..325 and the BOSS NAME plate at x 218..303, so sampling
// across the centre would stamp a little crest and a little name plate into
// the middle of every panel. The bar is uniform along its length, so any
// clean segment stretches identically.
const int  HUD_FRAME_M_X    = 60;
const int  HUD_FRAME_M_W    = 130;
const int  HUD_FRAME_R_X    = 487;
const char PATH_BG_MUSIC[] = "Audios/background.mp3";

// Played once when the run ends, one for each outcome. These replace the old
// Audios/gameover.mp3 sting, which is no longer referenced.
const char PATH_WIN_SOUND[]  = "win sound.mpeg";
const char PATH_LOSE_SOUND[] = "loose sound.mpeg";

const char PATH_CHARACTER_FMT[] = "characters_by_sequence/character_%d.png";
const char PATH_BG_FOLDER[]     = "backround images/";

// Obstacles. Each road type has a "coming" sprite that faces left and a
// "going" sprite that faces right, which is exactly how they travel.
const char PATH_BIKE_COMING[] = "game_assetes/bike_is_comming.jpg";
const char PATH_BIKE_GOING[]  = "game_assetes/bike_is_going.jpg";
const char PATH_CAR_COMING[]  = "game_assetes/car_is_coming.png";
const char PATH_CAR_GOING[]   = "game_assetes/car_is_going.png";
const char PATH_DOG_COMING[]  = "game_assetes/dog_is_coming.jpg";
const char PATH_DOG_GOING[]   = "game_assetes/dog_is_going.jpg";
const char PATH_RICK_COMING[] = "game_assetes/rickshaw_is_coming.jpg";
const char PATH_RICK_GOING[]  = "game_assetes/rickshaw_is_going.jpg";
const char PATH_BIRDS[]       = "game_assetes/Flying_birds.png";

// rickshaw_is_coming.jpg is a WATERMARKED STOCK PREVIEW: a heavy grey
// "shutterstock" mark is stamped right across the frame, down to about
// (61,61,69). No background cut can lift that off without destroying the
// rickshaw with it, so until the file is replaced with a clean one the
// "going" artwork is mirrored to face left and used for both directions.
// Replace the file, set this to false, and the real sprite comes back.
const bool RICKSHAW_COMING_NEEDS_FALLBACK = true;

const char PATH_COIN[]      = "game_assetes/coin.jpg";
const char PATH_PU_HEALTH[] = "game_assetes/powerup_health.png";
const char PATH_PU_SHIELD[] = "game_assetes/powerup_shield.png";

// Warning sounds. One per obstacle, all sitting in the solution root, so they
// resolve through assetPath(). The spaces in the names are real.
const char PATH_SFX_BIKE[] = "motocycle horn.wav";
const char PATH_SFX_CAR[]  = "car horn.wav";
const char PATH_SFX_DOG[]  = "dog barking sound.wav";
const char PATH_SFX_RICK[] = "rickshaw horn.wav";
const char PATH_SFX_BIRD[] = "birds chirping sound.wav";

// The narration that loops under the title screen until NEW GAME is pressed,
// and the thud that plays on every collision. Both are MP3 despite the .mpeg
// extension, which audioOpen handles by naming the MPEGVideo device.
const char PATH_INTRO_VOICE[]   = "introduction voice.mpeg";
const char PATH_SFX_COLLISION[] = "collision sound.mpeg";

// Where each clip sits in the mix, 0-1000. 1000 is as loud as MCI goes, so the
// collision thud stands out by everything else being held below it.
const int MIX_COLLISION = 1000;
const int MIX_WARNING   =  720;
const int MIX_INTRO     =  900;
const int MIX_ENDING    =  950;   // the win / lose sting
const int MIX_MUSIC     =  520;

const int CHARACTER_FRAME_COUNT = 6;   // character_1.png .. character_6.png

// ---------------------------------------------------------------------------
//  Level select / campaign map
//
//  The marker positions live in LevelSelect.hpp, written in the map image's own
//  pixel coordinates. Only the sizes that are fixed on screen live here.
//
//  The nominal size is the map as supplied (1536 x 1024). It is a fallback: the
//  real dimensions come from the loaded texture, so replacing the artwork with a
//  different size still lines everything up.
// ---------------------------------------------------------------------------
const double LEVELMAP_NOMINAL_W = 1536.0;
const double LEVELMAP_NOMINAL_H = 1024.0;

const double LS_BADGE_W = 168.0;   // the status plate under each pin
const double LS_BADGE_H = 50.0;

// How far above the badge the clickable area reaches, in MAP pixels, so it
// covers the pin standing on the pedestal and nothing else.
const double LS_PIN_REACH_IMG = 118.0;

// The title plate, in map pixels: the clear band between the map's own wooden
// sign and its mission panel.
const double LS_TITLE_IMG_X = 545.0;
const double LS_TITLE_IMG_Y = 148.0;
const double LS_TITLE_IMG_W = 600.0;
const double LS_TITLE_IMG_H = 118.0;

const double LS_BACK_X = 14.0;     // screen pixels, bottom-left
const double LS_BACK_Y = 14.0;
const double LS_BACK_W = 112.0;
const double LS_BACK_H = 40.0;

const double LS_TOAST_SECONDS = 2.6;
const double LS_TICK_SECONDS  = 0.016;   // fixedUpdate() cadence

// ---------------------------------------------------------------------------
//  Level 01  --  Mohanagar to Hatirjheel
// ---------------------------------------------------------------------------
const double LEVEL_TIME_LIMIT = 120.0;   // 2 minutes
const int    PLAYER_MAX_HP    = 200;
const int    PLAYER_START_HP  = 200;
const int    PLAYER_LIVES     = 1;       // one life, no second chance
const int    SHIELD_OBSTACLES = 2;       // shield absorbs the next 2 hits
const int    HEAL_AMOUNT      = 50;
const int    COIN_VALUE       = 10;

// Pixels per second. The character never moves along the street: this is how
// fast the street moves past.
const double RUN_SPEED = 210.0;

// A clean run takes about 100 seconds, inside the 2-minute limit with room to
// spare for the odd stumble.
const double TARGET_RUN_SECONDS = 100.0;

// ---------------------------------------------------------------------------
//  The single running lane
//
//  One ground line. The character sits far enough right that traffic entering
//  from the left still has room to be seen and heard before it arrives.
// ---------------------------------------------------------------------------
const double GROUND_Y = 44.0;
const double PLAYER_X = 400.0;

const double PLAYER_BASE_H     = 172.0;
const double PLAYER_ASPECT     = 358.0 / 554.0;
const double PLAYER_FRAME_TIME = 0.085;    // running animation, ~12 fps

// Jump, as velocity against gravity.
//   apex     = JUMP_VELOCITY^2 / (2 * GRAVITY)   = 190 px
//   air time = 2 * JUMP_VELOCITY / GRAVITY       = 0.90 s
const double JUMP_VELOCITY = 846.0;
const double GRAVITY       = 1880.0;

const double INVULN_TIME = 1.10;

// ---------------------------------------------------------------------------
//  Obstacles
// ---------------------------------------------------------------------------
// Exactly one obstacle exists at a time -- birds included, so the game can
// never demand a jump and a duck at once. The gap is counted from the moment
// the previous one leaves the screen.
const double MIN_OBSTACLE_INTERVAL = 2.4;
const double MAX_OBSTACLE_INTERVAL = 4.0;

// How close an obstacle gets before its own sound plays. Roughly two seconds
// of warning at these speeds.
const double WARNING_DISTANCE = 640.0;

// ---------------------------------------------------------------------------
//  The last minute
//
//  For most of the run exactly one obstacle is on the road at a time. Inside
//  the final minute the street turns: the gap collapses, traffic speeds up, and
//  up to three things can be on screen at once, so the approach to Hatirjheel
//  is a scramble.
//
//  RUSH_MIN_SEPARATION is what keeps it a scramble rather than a coin flip.
//  Nothing new is released until the previous obstacle has put that many pixels
//  between itself and the edge, which guarantees room to land a jump and take
//  off again before the next one arrives.
// ---------------------------------------------------------------------------
const double FINAL_RUSH_SECONDS  = 60.0;
const double RUSH_MIN_INTERVAL   = 0.35;
const double RUSH_MAX_INTERVAL   = 0.95;
const double RUSH_SPEED_SCALE    = 1.20;
const double RUSH_MIN_SEPARATION = 470.0;

const int MAX_ROAD_OBSTACLES = 4;   // slots; only 1 is used outside the rush
const int RUSH_MAX_ACTIVE    = 3;

// Drawn heights. Jumping is the only way past anything, and the jump apex is
// 190 px, so these are deliberately well under it: at the top of a jump the
// character's box clears a rickshaw by about 100 px, which leaves a wide,
// forgiving window rather than a frame-perfect one.
const double OB_H_BIKE     =  92.0;
const double OB_H_CAR      =  74.0;
const double OB_H_DOG      =  76.0;
const double OB_H_RICKSHAW = 104.0;
const double OB_H_BIRD     =  96.0;

// Travel speed, and it has to be read together with the jump.
//
// An obstacle is dodgeable only if it spends LESS time overlapping the
// character's x-range than the jump spends high enough to clear it. That time
// is (obstacleWidth + characterWidth) / relativeSpeed, so a wide obstacle
// moving slowly is the dangerous combination -- not a tall one.
//
// "Going" traffic used to creep past at 250, which put a car in the way for
// 0.91 s against a 0.90 s jump: literally impossible, every single time. It now
// matches the oncoming speed, which still leaves about two seconds of warning
// after it enters from the left.
const double OB_SPEED_COMING = 330.0;
const double OB_SPEED_GOING  = 330.0;
const double OB_SPEED_BIRD   = 300.0;

// Damage. Every collision costs the same, whatever hit: 25 off 200, so eight
// hits end the run. The per-type constants are kept so they can be split apart
// again later -- change COLLISION_DAMAGE to move them all at once.
const int COLLISION_DAMAGE = 25;

const int DMG_BIKE     = COLLISION_DAMAGE;
const int DMG_CAR      = COLLISION_DAMAGE;
const int DMG_DOG      = COLLISION_DAMAGE;
const int DMG_RICKSHAW = COLLISION_DAMAGE;
const int DMG_BIRD     = COLLISION_DAMAGE;

// Birds fly in one of two bands. The low one has to be jumped over; the high
// one passes safely overhead unless the character jumps into it.
// The low band had the same problem as the car: the flock is wide, so at 62 px
// up it needed more lift than the jump could hold. Dropping it nearer the
// ground shortens the lift needed and makes it clearable.
const double BIRD_LOW_Y  =  34.0;    // above the ground line
const double BIRD_HIGH_Y = 216.0;

// ---------------------------------------------------------------------------
//  Coins
// ---------------------------------------------------------------------------
const int    MAX_COINS         = 14;
const int    COINS_PER_ROW_MIN = 3;
const int    COINS_PER_ROW_MAX = 5;
const double COIN_SPACING      = 84.0;
const double COIN_SIZE         = 52.0;
const double COIN_HEIGHT       = 66.0;    // above the ground, in the body path
const double MIN_COIN_INTERVAL = 2.0;
const double MAX_COIN_INTERVAL = 4.0;

// ---------------------------------------------------------------------------
//  Power-ups
// ---------------------------------------------------------------------------
const int    MAX_PICKUPS         = 2;
const double MIN_PICKUP_INTERVAL = 15.0;
const double MAX_PICKUP_INTERVAL = 24.0;
const double PICKUP_SIZE         = 92.0;
const double PICKUP_HEIGHT       = 70.0;

// The finish gate slides in over the last stretch of road.
const double FINISH_APPROACH = 1400.0;

// ---------------------------------------------------------------------------
//  Level 02  --  the river crossing  (PART 1 ONLY)
//
//  Everything below belongs to Level 02 and is read by nothing above it, so
//  Level 01 is untouched by any of it.
//
//  The level02 folder sits in the solution root beside game_poster.jpeg, so
//  these paths resolve through the same assetPath(). The names are EXACTLY as
//  they are on disk -- "rain sound.wav" really does have a space in it, the
//  boats really are .jpeg while boatstand really is .png.
// ---------------------------------------------------------------------------
const char PATH_L02_BG[]         = "level02/level02_bg.png";
const char PATH_L02_BOAT_1[]     = "level02/boatPic01.jpeg";
const char PATH_L02_BOAT_2[]     = "level02/boatPic02.jpeg";
const char PATH_L02_BOAT_STAND[] = "level02/boatstand.png";
const char PATH_L02_BOAT_THROW[] = "level02/throwpic.png";
const char PATH_L02_CHIDORI[]    = "level02/chidori.png";
const char PATH_L02_DRAGON[]     = "level02/dragonPower.jpg";
const char PATH_L02_CROCODILE[]  = "level02/crocodile_image.jpg";
const char PATH_L02_COIN[]       = "level02/coin.jpg";

const char PATH_L02_RAIN[]     = "level02/rain sound.wav";
const char PATH_L02_CROC_SFX[] = "level02/crocodile sound.mp3";

// The rain runs under the whole level, so it sits well down in the mix. The
// crocodiles have to be heard over it without drowning the collision thud.
const int MIX_RAIN = 380;
const int MIX_CROC = 720;

// The health bar for this level is its own number, not PLAYER_MAX_HP: the river
// is longer and the crocodiles bite more than once each.
const int LEVEL02_INITIAL_HP = 700;

const double L02_READY_SECONDS = 2.4;

// ---------------------------------------------------------------------------
//  Stage timing
// ---------------------------------------------------------------------------
const double BOAT_TRAVEL_DURATION     = 11.0;   // rowing before the ball arrives
const double L02_BOAT_STOPPED_SECONDS = 1.9;    // the beat after standing up
const double L02_GROUP_GAP_SECONDS    = 1.5;    // between crocodile groups

// ---------------------------------------------------------------------------
//  The river
//
//  L02_WATERLINE is where the far bank meets the water in level02_bg.png,
//  measured off the artwork: 145 of 339 sampled rows, which is y = 289 once the
//  backdrop is stretched to the 675-pixel window. Everything that floats is
//  placed below it.
// ---------------------------------------------------------------------------
const double L02_WATERLINE      = 289.0;
const double RIVER_SCROLL_SPEED = 130.0;   // px/s while the boat is under way

// ---------------------------------------------------------------------------
//  The boat
//
//  The boy is drawn INSIDE every boat sprite, so there is no separate character
//  to composite -- the boat rect is the character rect.
//
//  All four poses are anchored by their BOTTOM edge at the same waterline and
//  share one width, so the hull stays put while the boy sits, stands and
//  throws. The cut-out aspects differ (rowing 2.34, standing 1.75), which is
//  exactly why: at a shared width, standing up makes him taller, as it should.
// ---------------------------------------------------------------------------
const double BOAT_X = 150.0;
const double BOAT_Y =  74.0;
const double BOAT_W = 430.0;

const double BOAT_BOB            =   7.0;   // half the bob, in pixels
const double BOAT_BOB_SPEED      =   1.9;   // radians per second
const double BOAT_ROW_FRAME_TIME =   0.30;  // seconds per oar stroke frame

// ---------------------------------------------------------------------------
//  The Dragon Power ball
//
//  A real projectile, not a cut: it is spawned off the right edge, travels
//  left, and the stage only changes when its box actually touches the boat.
//  It is a story beat, so it costs no health.
// ---------------------------------------------------------------------------
const double DRAGON_SIZE      = 132.0;
const double DRAGON_SPEED     = 430.0;

// The height it flies at, and it has to be read against the boat's box, not
// picked by eye. The boat sits at BOAT_Y = 74 and is about 183 tall, and its
// hit box is the middle 78% of that, so it reaches from roughly y = 92 to
// y = 235. At 232 the ball's own box started at 249 and sailed clean over the
// top of him, and the stage never advanced. At 132 the two boxes overlap
// across y = 149..235, which is squarely the boy and the hull.
const double DRAGON_Y         = 132.0;
const double DRAGON_PULSE     =   6.0;   // radians per second
const double DRAGON_FLASH     =   0.55;  // white flash held after impact

// ---------------------------------------------------------------------------
//  Crocodiles  --  four of them, in two groups of two
//
//  crocodile_image.jpg already faces LEFT, which is the direction they swim, so
//  it is drawn unflipped.
//
//  Each takes exactly CROCODILE_HITS_TO_KILL chidori. Once one reaches the boat
//  it stops and bites on a cooldown rather than draining the bar every frame.
// ---------------------------------------------------------------------------
const int MAX_CROCODILES  = 4;
const int CROC_GROUP_SIZE = 2;

const int    CROCODILE_HITS_TO_KILL     = 3;
const int    CROCODILE_COLLISION_DAMAGE = 20;
const double CROCODILE_DAMAGE_COOLDOWN  = 1.2;

const double CROC_W         = 196.0;
const double CROC_Y_LOW     =  52.0;   // the two in a group swim at
const double CROC_Y_HIGH    = 118.0;   // different depths so neither hides
const double CROC_SPEED_MIN =  64.0;
const double CROC_SPEED_MAX =  94.0;
const double CROC_SPAWN_GAP = 240.0;   // between the two of a group
const double CROC_STOP_GAP  =  10.0;   // how close it gets before it bites
const double CROC_KNOCKBACK =  52.0;   // shoved back by each chidori
const double CROC_HIT_FLASH =   0.22;
const double CROC_DEATH_TIME =  0.55;  // sinking, before it is gone

// ---------------------------------------------------------------------------
//  Chidori
//
//  chidori.png ships with real alpha, so it is loaded without a background cut.
//  One projectile is one hit: it deactivates the instant it connects.
//
//  The throw is a three-step animation -- standing, hand raised (throwpic), the
//  chidori leaves the hand, back to standing. HAND_FX/FY locate the boy's hand
//  as a fraction of the boat rect, which is how the bolt leaves the right place
//  whatever size the boat is drawn at.
// ---------------------------------------------------------------------------
const int    MAX_CHIDORI      = 6;
const double CHIDORI_W        = 150.0;
const double CHIDORI_SPEED    = 760.0;
const double CHIDORI_WINDUP   = 0.16;   // hand raised before it is released
const double CHIDORI_RECOVER  = 0.16;   // hand still raised after
const double CHIDORI_COOLDOWN = 0.40;
const double CHIDORI_LIFE     = 2.0;

const double HAND_FX = 0.52;   // across the boat rect
const double HAND_FY = 0.62;   // up the boat rect

// ---------------------------------------------------------------------------
//  Coins on the water  --  score only, exactly as in Level 01
// ---------------------------------------------------------------------------
const int    MAX_L02_COINS         = 6;
const double L02_COIN_SIZE         = 54.0;
const double L02_COIN_MIN_INTERVAL = 1.5;
const double L02_COIN_MAX_INTERVAL = 3.0;
const double L02_COIN_DRIFT        = 215.0;
const double L02_COIN_Y_LOW        = 120.0;
const double L02_COIN_Y_HIGH       = 235.0;

// Cosmetic rain, to match the rain that is playing and the storm in the
// backdrop. Set RAIN_STREAKS to 0 to turn it off entirely.
const int    RAIN_STREAKS   = 90;
const double RAIN_FALL      = 900.0;
const double RAIN_SLANT     = 150.0;
const double RAIN_LENGTH    = 22.0;

// ---------------------------------------------------------------------------
//  Level 02  --  SECOND HALF: the pirate ship
//
//  Picks up the moment the fourth crocodile dies. He rows on, a pirate ship
//  rams the boat, and the rest of the level happens on that ship's deck: six
//  dakat with swords, then their leader.
//
//  Every name below was read off the folder, not guessed. The ones that catch
//  people out:
//      "boatside view.jpg"     the space is real
//      "pirates ship.png"      plural pirates, and a 4-bit indexed PNG with a
//                              white/grey chequerboard baked in
//      "pirate song.mpeg"      an MP3 wearing a .mpeg extension, like the rest
//      "dakatleader.jpeg"      .jpeg, not .jpg
//      "enemy chidori.jpg"     the space is real
//      "powerfulchidori.png"   see the note on the AVIF below
// ---------------------------------------------------------------------------
const char PATH_L02B_DECK[]      = "level02/boatside view.jpg";
const char PATH_L02B_SHIP[]      = "level02/pirates ship.png";
const char PATH_L02B_THROW[]     = "level02/throw.png";

// The replacement art arrived as .jpeg, not .png -- fighter_01.png and its two
// neighbours are no longer on disk at all, so these paths had to follow the
// files. Nothing else about the dakat changed.
const char PATH_L02B_FIGHTER1[]  = "level02/fighter_01.jpeg";
const char PATH_L02B_FIGHTER2[]  = "level02/fighter_02.jpeg";
const char PATH_L02B_FIGHTER3[]  = "level02/fighter_03.jpeg";

const char PATH_L02B_ICON[]      = "level02/icon.jpg";
const char PATH_L02B_POWERUP[]   = "level02/powerup.png";
const char PATH_L02B_LEADER[]    = "level02/dakatleader.jpeg";

// The bolt he throws at the dakat. See the note below on why this is
// chidori.png and not throw.png.
const char PATH_L02B_CHIDORI[]   = "level02/chidori.png";
const char PATH_L02B_ENEMY_CHI[] = "level02/enemy chidori.jpg";

// powerfulchidori is supplied as .avif, and stb_image -- which is the whole of
// this project's image loading -- has no AVIF decoder, so the file simply
// cannot be opened. A PNG copy is written beside it and loaded instead. The
// .avif is left exactly where it was; nothing was renamed or deleted.
const char PATH_L02B_POWERFUL[]  = "level02/powerfulchidori.png";

const char PATH_L02B_PIRATE_SONG[] = "level02/pirate song.mpeg";

// Played when Level 02 is won, over the Hatirjheel boat scene. The file really
// is called "loose sound2" -- it lives in level02/ and it is what the design
// asks for on the WIN, whatever its name suggests.
const char PATH_L02B_WIN_SONG[] = "level02/loose sound2.mpeg";

// The pirate song carries the whole second half, so it sits where the river's
// rain used to: present, but under everything that matters.
const int MIX_PIRATE = 480;
const int MIX_L02_WIN = 900;

// ---------------------------------------------------------------------------
//  Scene 1  --  rowing on, and the ram
// ---------------------------------------------------------------------------
const double L02_AFTER_CROC_SECONDS = 2.8;   // he sits back down and rows
const double SHIP_H       = 470.0;           // big enough to read as a ship
const double SHIP_Y       =  92.0;
const double SHIP_SPEED   = 300.0;
const double SHIP_START_X = 1320.0;
const double PIRATE_HIT_HOLD = 1.6;          // the crash, before the cut inside

// ---------------------------------------------------------------------------
//  Scene 2  --  the deck
//
//  boatside view.jpg is a perspective interior: sampling its rows shows the lit
//  wooden deck running from the bottom of the frame up to about y = 200 in game
//  coordinates, with the shadowed far opening above that. So everything that
//  stands on the deck stands along the bottom of the frame.
//
//  Unlike Level 01 he is NOT locked to one spot here. The boss throws a
//  projectile that has to be dodged and the pickups have to be walked into, so
//  he is driven left and right between two walls, and he can jump.
// ---------------------------------------------------------------------------
const double L02B_GROUND_Y   =  46.0;
const double L02B_START_X    =  90.0;
const double L02B_WALK_SPEED = 210.0;
const double L02B_MIN_X      =  30.0;    // he cannot back off the deck
const double L02B_MAX_X      = 700.0;    // nor walk into the boss's lap

// throw.png is cut down to its content while the six walk frames keep their
// whole canvas, so the two are not framed alike. This trims the throw pose to
// match the walk cycle's visible height.
const double L02B_THROW_H_SCALE = 0.97;

// The jump. Same shape as Level 01's -- an upward velocity fought by gravity,
// integrated every frame -- but tuned so that jumping over a dakat actually
// works, which it has to be MEASURED to do rather than assumed:
//
//   apex     = v^2 / (2g) = 195 px
//   air time = 2v / g     = 0.95 s
//
//   A dakat's box now reaches 117 px above the floor, and his own box starts
//   7 px above his feet, so he is clear of it while he is more than 110 px up.
//   Solving 820t - 860t^2 > 110 gives him 0.63 s of clearance.
//
//   Running into a dakat closes at 210 + 90 = 300 px/s, and the two boxes are
//   72 + 56 = 128 px wide together, so they overlap for 0.43 s. 0.43 < 0.63, so
//   jumping THROUGH one clears it with room to spare.
//
//   At the old 760 the clearance window was 0.40 s against the old, taller box,
//   and the jump could not be made at all.
const double L02B_JUMP_VELOCITY = 820.0;
const double L02B_JUMP_GRAVITY  = 1720.0;

// ---------------------------------------------------------------------------
//  The chidori icon  --  what makes an attack possible
//
//  He cannot throw at will. Walking into the icon buys him a fixed number of
//  chidori, and when those run out another icon appears somewhere on the deck
//  and has to be walked into as well.
// ---------------------------------------------------------------------------
const double L02B_ICON_H = 92.0;
const double L02B_ICON_Y = 118.0;      // at his chest, so walking in connects

const double L02B_ICON_X_FIRST = 330.0;   // where the very first one waits
const int    CHIDORI_PER_ICON  = 6;    // two dakat's worth, if nothing misses
const double ICON_RESPAWN_WAIT = 1.1;  // beat before the next one appears
const double ICON_MIN_X = 240.0;       // where a new icon may stand
const double ICON_MAX_X = 660.0;

// ---------------------------------------------------------------------------
//  The dakat
//
//  Six of them, drawn from the three fighter sprites in rotation. They are on
//  the deck two at a time so it never turns into a wall of swords, and the
//  sixth has to die before the powerup will appear.
//
//  fighter_01..03 already face LEFT, which is the way they advance, so they are
//  drawn unflipped going that way and mirrored when they turn back.
// ---------------------------------------------------------------------------
const int DAKAT_TOTAL   = 6;    // exactly six, and the boss waits for all six
const int MAX_DAKAT     = 2;    // on the deck at once

const double DAKAT_H = 150.0;
const int    DAKAT_HITS_TO_KILL   = 3;   // each keeps its OWN counter
const int    DAKAT_COLLISION_DAMAGE = 10;
const double DAKAT_DAMAGE_COOLDOWN  = 1.1;

const double DAKAT_SPEED_MIN  =  76.0;
const double DAKAT_SPEED_MAX  = 104.0;
const double DAKAT_SPAWN_GAP  = 190.0;
const double DAKAT_KNOCKBACK  =  40.0;
const double DAKAT_HIT_FLASH  =   0.22;
const double DAKAT_DEATH_TIME =   0.45;
const double DAKAT_NEXT_WAIT  =   0.9;   // before the next one walks on
// Where a dakat's FEET go. Everything on this deck stands on the same floor as
// the character -- L02B_GROUND_Y -- and these two lines sit a little either
// side of it. They are the depth of the actor, and they are what decides both
// the sprite's bottom edge AND what is drawn in front of what.
//
// The old pair were 40 and 84 against a character standing at 46, so half the
// dakat spawned 38 px above the floor he was walking on and hung in the air.
// Twelve pixels either side reads as depth on a receding deck without any of
// them leaving the ground.
const double DAKAT_Y_NEAR = L02B_GROUND_Y - 12.0;   // in front of him
const double DAKAT_Y_FAR  = L02B_GROUND_Y + 12.0;   // behind him

// A dakat that walks past him does NOT vanish -- it turns round at the wall and
// comes back. Jumping over one is a dodge, never a way to skip it, so the count
// of six can never be cheated.
const double DAKAT_TURN_X = -40.0;

// How far clear of HIM a dakat gets before it turns and comes back. Measured
// from his body box, not from the screen, so the fight stays around him
// wherever on the deck he happens to be standing.
const double DAKAT_TURN_CLEARANCE = 120.0;

// ---------------------------------------------------------------------------
//  The chidori he throws
// ---------------------------------------------------------------------------
const double L02B_CHIDORI_W     = 120.0;
const double L02B_CHIDORI_SPEED = 760.0;

const double L02B_THROW_WINDUP   = 0.15;
const double L02B_THROW_RECOVER  = 0.17;
const double L02B_THROW_COOLDOWN = 0.34;

// His hand in throw.png, as a fraction of the drawn rect.
const double L02B_HAND_FX = 0.86;
const double L02B_HAND_FY = 0.62;

// ---------------------------------------------------------------------------
//  The powerup, and the powerful chidori it grants
// ---------------------------------------------------------------------------
const double L02B_POWERUP_H = 118.0;
const double L02B_POWERUP_X = 560.0;
const double L02B_POWERUP_Y = 112.0;

// Deliberately much larger than the ordinary bolt: this is the final attack and
// it has to read as one.
const double POWERFUL_W     = 230.0;
const double POWERFUL_SPEED = 700.0;
const double POWERFUL_COOLDOWN = 0.46;

// ---------------------------------------------------------------------------
//  Dakatleader  --  the boss
// ---------------------------------------------------------------------------
// The replacement dakatleader is a wider composition -- its cut-out content
// went from 1.20 to 1.48 wide-to-tall, because there is more empty deck either
// side of him. At the old height of 330 that made the sprite 489 wide and his
// own figure ran 26 px off the right edge of the window. 300 brings the whole
// figure back on screen at the same LEADER_X, and he is still twice a dakat.
const double LEADER_H = 300.0;      // dominant: twice a dakat and then some
const double LEADER_X = 840.0;
const double LEADER_Y = L02B_GROUND_Y;   // the same floor as everyone else
const double LEADER_ENTRY_SECONDS = 1.4;
const double LEADER_BOB_SPEED = 1.2;
const double LEADER_BOB = 8.0;
const double LEADER_DEATH_TIME = 1.6;

// ---------------------------------------------------------------------------
//  Back to Hatirjheel
//
//  Beating the dakatleader does not end on the deck. The pirate song stops, the
//  scene returns to the river he was rowing before the ship rammed him, and he
//  rows on for a moment before the win message appears -- so the level ends
//  where it began rather than on a banner over a corpse.
// ---------------------------------------------------------------------------
const double L02_RETURN_ROW_SECONDS = 2.2;

const int LEADER_HITS_TO_KILL = 6;  // six powerful chidori, and not one fewer

// His answer: enemy chidori, thrown on a timer, and it hurts.
const int    MAX_ENEMY_SHOTS = 4;
const double ENEMY_CHI_H     = 170.0;
const double ENEMY_CHI_SPEED = 300.0;
const int    ENEMY_CHI_DAMAGE = 75;         // exactly 75, once per projectile
const double ENEMY_CHI_INTERVAL_MIN = 2.0;
const double ENEMY_CHI_INTERVAL_MAX = 3.2;

// ===========================================================================
//  LEVEL 03  --  Kunipara to AUST
//
//  A long walk through fifteen backdrops with five set-pieces along it: a
//  protest march, a ragging confrontation, a group of eve-teasers, a thief,
//  and finally a jam of parked traffic to run across under falling fire.
//
//  Street traffic crosses the road the whole way up to that last phase and has
//  to be jumped, exactly as in Level 01.
//
//  Every name below was read off the level03 folder, not guessed. The ones
//  that catch people out:
//      "backround/"            spelled that way, and inside level03
//      "bike_is_comming.jpg"   double m, as in Level 01
//      "rOAD BARRIER.png"      the capitals really are like that (unused)
// ===========================================================================
const char PATH_L03_BG_FOLDER[]  = "level03/backround/";
const char PATH_L03_CHAR_FMT[]   = "level03/characters_by_sequence/character_%d.png";
const char PATH_L03_RUN_FMT[]    = "level03/run_frames/run_%02d.png";
const char PATH_L03_TALK_FMT[]   = "level03/audios/%d.ogg";

const char PATH_L03_THROW[]      = "level03/throw.png";
const char PATH_L03_ROCK[]       = "level03/rock.png";
const char PATH_L03_PROTEST[]    = "level03/protest.png";
// The michil's own two: the petrol bomb it lobs, and the portrait frame it
// guards behind. Both are .jpg with a white studio background, cut at load.
// The spaces and the capital M are exactly as the files are named on disk.
const char PATH_L03_COCKTAIL[]   = "level03/Michil cocktail.jpg";
const char PATH_L03_SHIELD[]     = "level03/Michil shield.jpg";
const char PATH_L03_RAGGING[]    = "level03/ragging.jpg";
const char PATH_L03_STICK[]      = "level03/stick.jpg";
const char PATH_L03_EVETEASE[]   = "level03/eveteasing.jpg";
// The one who breaks off and charges. A separate file on purpose: the boys
// in eveteasing.jpg overlap, so none of them can be cut out cleanly, and the
// group has to stay whole behind him anyway.
const char PATH_L03_EVE_RUSHER[] = "level03/eve_aggressor.jpg";
const char PATH_L03_BEG[]        = "level03/beg.png";
const char PATH_L03_SLAP1[]      = "level03/boy_01_with_fx.png";
const char PATH_L03_SLAP2[]      = "level03/boy_02.png";
const char PATH_L03_FLAME[]      = "level03/flame.png";
const char PATH_L03_COIN[]       = "level03/coin.jpg";

const char PATH_L03_BUS[]        = "level03/bus.png";
const char PATH_L03_CAR01[]      = "level03/car01.jpg";
const char PATH_L03_CAR02[]      = "level03/car02.jpg";
const char PATH_L03_CAR03[]      = "level03/car03.jpg";
const char PATH_L03_CNG[]        = "level03/cng.jpg";
const char PATH_L03_RICKSHAW[]   = "level03/rickshaw.png";

// The street traffic that has to be jumped. Level 03 has its own copies.
const char PATH_L03_BIKE_COMING[] = "level03/bike_is_comming.jpg";
const char PATH_L03_BIKE_GOING[]  = "level03/bike_is_going.jpg";
const char PATH_L03_CAR_COMING[]  = "level03/car_is_coming.png";
const char PATH_L03_CAR_GOING[]   = "level03/car_is_going.png";
const char PATH_L03_RICK_COMING[] = "level03/rickshaw_is_coming.jpg";
const char PATH_L03_RICK_GOING[]  = "level03/rickshaw_is_going.jpg";

// The AUST gate. Not part of the scrolling road: it is the one backdrop the
// street never reaches, shown only once the jam has been crossed.
const char PATH_L03_BG_LAST[]    = "level03/bg_last.jpeg";

const char PATH_L03_MICHIL_SFX[] = "level03/michil.mpeg";
const char PATH_L03_BEG_SFX[]    = "level03/beg.mpeg";

const int MIX_MICHIL = 780;
const int MIX_BEG    = 850;
const int MIX_TALK   = 950;   // the conversation has to be clearly audible

// ---------------------------------------------------------------------------
//  Health
//
//  Every hit in this level costs 50, from a street vehicle or from a falling
//  flame alike. 600 is twelve mistakes, and it needs to be that generous: the
//  road is long, five set-pieces happen on it, traffic crosses through ALL of
//  them -- including the conversation, where he is standing still -- and the
//  minute on the jam is on top of all that. At 300 a player was down to 50
//  health before the michil had even finished walking on.
// ---------------------------------------------------------------------------
const int    LEVEL03_INITIAL_HP = 800;
const int    L03_HIT_DAMAGE     = 50;

// The immunity window after a hit. It is also the real brake on how fast the
// bar can empty: at 1.10 s a bad patch on the jam took him from full to zero
// in four seconds, because a second flame was always already on him.
const double L03_HIT_COOLDOWN   = 1.60;
const double L03_READY_SECONDS  = 2.4;

// ---------------------------------------------------------------------------
//  The walk
//
//  He runs on the spot at a fixed x and the street moves past, exactly as in
//  Level 01 -- until the traffic phase, where he takes control of his own x.
// ---------------------------------------------------------------------------
const double L03_GROUND_Y   = 44.0;
const double L03_PLAYER_X   = 330.0;
const double L03_WALK_SPEED = 205.0;

const double L03_JUMP_VELOCITY = 880.0;   // apex 206 px, air time 0.94 s
const double L03_JUMP_GRAVITY  = 1880.0;

// How far he walks before each set-piece, in pixels of road.
const double L03_DIST_ROCK    = 1100.0;
const double L03_DIST_MICHIL  = 2100.0;
const double L03_DIST_RAGGING = 3400.0;
const double L03_DIST_EVE     = 4700.0;
const double L03_DIST_CHOR    = 5900.0;
const double L03_DIST_TRAFFIC = 7100.0;

// ---------------------------------------------------------------------------
//  Street traffic  --  jump or be hit
// ---------------------------------------------------------------------------
// Two on the road at once at the very most, and a real gap between them.
// Three at a time with a 2.3 s gap meant something was always on top of him
// and jumping could not keep up.
const int    MAX_L03_TRAFFIC   = 2;
const double L03_TRAFFIC_MIN_GAP = 3.0;
const double L03_TRAFFIC_MAX_GAP = 5.0;
const double L03_TRAFFIC_SPEED   = 330.0;

// Drawn heights, kept under the jump apex so every one of them is clearable.
const double L03_H_BIKE = 92.0;
const double L03_H_CAR  = 78.0;
const double L03_H_RICK = 104.0;

// ---------------------------------------------------------------------------
//  Pickups  --  the rock, then the stick
// ---------------------------------------------------------------------------
const double L03_PICKUP_H = 86.0;
const double L03_PICKUP_Y = 96.0;

// ---------------------------------------------------------------------------
//  Throwing
// ---------------------------------------------------------------------------
const double L03_SHOT_W      = 78.0;
const double L03_SHOT_SPEED  = 760.0;
const int    MAX_L03_SHOTS   = 6;

const double L03_THROW_WINDUP   = 0.15;
const double L03_THROW_RECOVER  = 0.17;
const double L03_THROW_COOLDOWN = 0.34;

const double L03_HAND_FX = 0.86;   // his hand in throw.png
const double L03_HAND_FY = 0.62;

// ---------------------------------------------------------------------------
//  Set-piece 1  --  the michil
// ---------------------------------------------------------------------------
const int    MICHIL_HITS_TO_CLEAR = 7;    // seven rocks, and not six
const double MICHIL_H     = 300.0;        // deliberately bigger than a person
const double MICHIL_Y     =  40.0;
// It used to creep in at 46, which is sixteen seconds from off-screen to its
// holding distance -- bearable when rocks landed on the way, unbearable now
// the shield eats them. It also governs how fast it recovers the ground a
// landed rock knocks it back.
const double MICHIL_SPEED = 130.0;
const double MICHIL_STOP_X = 560.0;       // how close it presses before it holds
const double MICHIL_KNOCKBACK = 34.0;
const double MICHIL_HIT_FLASH = 0.22;
const double MICHIL_DEATH_TIME = 1.0;

// ---------------------------------------------------------------------------
//  The michil's attack cycle
//
//      GUARD -> WARNING -> ATTACK -> VULNERABLE -> GUARD
//
//  Behind its shield the crowd cannot be hurt: rocks that reach it are blocked
//  and spent. It only opens up for a moment after throwing, and that window is
//  the only time a rock counts. Seven counted hits still clear it.
//
//  THE TIMING IS THE WHOLE POINT, so it is worked out here rather than guessed:
//
//    Dodging.  The jump is 880 up against 1880 of gravity: 0.936 s in the air,
//    206 px at the apex. The cocktail's box tops out 108 px up, and his own box
//    clears that from 0.07 s to 0.866 s into a jump -- a 0.80 s window. The
//    cocktail is given a fixed FLIGHT TIME rather than a fixed speed, so the
//    interval from "JUMP!" to impact is always WARN + 0.45 s however far back
//    the crowd has been shoved. At the normal 0.60 s warning that is 1.05 s,
//    so any jump begun between 0.18 s and 0.98 s after the cue clears it; at
//    the hurried 0.50 s warning, between 0.08 s and 0.88 s. Both leave room
//    for an ordinary 0.2-0.4 s reaction.
//
//    Throwing.  A throw is 0.15 s of windup and roughly 0.2 s of rock flight,
//    so about 0.35 s from pressing F to the rock arriving. A 1.10 s window
//    means the key has to be down within about 0.75 s of "THROW NOW!".
// ---------------------------------------------------------------------------
// Where the fight starts: its holding distance, so every cocktail is launched
// from the same place. Engaging further out made the early bottles travel 435
// px instead of 150 in the same fixed flight time -- fair, because the warning
// is what you react to, but far too fast to read.
const double MICHIL_ENGAGE_X       = MICHIL_STOP_X + 4.0;

const double MICHIL_GUARD_TIME     = 1.55;   // shield up, nothing counts
const double MICHIL_WARN_TIME      = 0.60;   // red "JUMP!" before the throw
const double MICHIL_WARN_TIME_FAST = 0.50;   // once it is rattled -- see below
const double MICHIL_ATTACK_TIME    = 0.25;   // the throw itself, shield still up
const double MICHIL_VULN_TIME      = 1.10;   // shield down: the only scoring window

// Up to this many hits it always uses the full warning. Past it, some cycles
// hurry the warning to MICHIL_WARN_TIME_FAST. Still one cocktail per cycle,
// still dodgeable -- it just asks for a quicker read.
const int    MICHIL_RAGE_AFTER     = 4;
const int    MICHIL_RAGE_PERCENT   = 45;     // chance of the hurried warning

// How many rocks one open window is worth before the crowd closes up again.
//
// Without this the whole encounter collapsed into two windows: a player
// leaning on the throw key banked four hits in the first opening and three in
// the second, so "seven hits" really meant "survive two cocktails", and the
// hurried warning above -- which only exists past four hits -- almost never
// got a chance to appear. Capping the window sends the shield straight back
// up on the second hit, which the player SEES, and spreads the fight over
// about four cycles instead of two.
const int    MICHIL_HITS_PER_WINDOW = 2;

const double MICHIL_BLOCK_FLASH    = 0.45;   // how long "BLOCKED!" stays up

// The cocktail. Flight time is fixed; the speed follows from the distance.
const double MICHIL_COCKTAIL_H      = 58.0;
const double MICHIL_COCKTAIL_FLIGHT = 0.45;
const double MICHIL_COCKTAIL_Y      = L03_GROUND_Y + 6.0;   // ground level
const int    MICHIL_COCKTAIL_DAMAGE = 40;

// The shield frame, sized off the crowd it stands in front of. The height is
// scaled and the width follows the artwork's own aspect, so the portraits are
// never stretched. MICHIL_SHIELD_CX places it along the crowd: the march is
// far wider than the window, and its LEFT edge is the face the player throws
// at, so the frame sits over that rather than over the middle of the mob.
const double MICHIL_SHIELD_H_SCALE = 1.06;
const double MICHIL_SHIELD_CX      = 0.26;

// ---------------------------------------------------------------------------
//  Set-piece 2  --  the ragging conversation
//
//  Twenty-two clips in level03/audios, played strictly in order. Odd-numbered
//  lines are the raggers, even-numbered ones are the character, so the two
//  speak alternately and the next line only begins when the last has finished.
// ---------------------------------------------------------------------------
const int    TALK_LINE_COUNT = 22;
const double TALK_GAP        = 0.35;   // breath between lines
const double RAGGING_H       = 380.0;
const double RAGGING_X       = 700.0;
const double RAGGING_Y       =  44.0;

// ---------------------------------------------------------------------------
//  Set-piece 3  --  the eve-teasers
// ---------------------------------------------------------------------------
const int    EVE_HITS_TO_CLEAR = 5;    // five sticks
const double EVE_H     = 330.0;
const double EVE_X     = 690.0;
const double EVE_Y     =  44.0;
const double EVE_HIT_FLASH = 0.22;
const double EVE_DEATH_TIME = 0.9;

// ---------------------------------------------------------------------------
//  The eve-teasers fight back
//
//      GUARD -> WARNING -> RUSH -> RECOVERY -> GUARD
//
//  The GROUP artwork never moves: the girl they are surrounding is not the
//  attacker and is never the stick's target. One of them -- drawn separately,
//  see l03DrawRusher() -- breaks off, crouches, and rushes low along the
//  ground. Jump it. He then backs off to his spot and is open for a moment,
//  and only a stick that lands in that moment counts. Five counted hits still
//  clear the encounter.
//
//  THE ARITHMETIC, because the whole thing stands or falls on it:
//
//    Dodging. The jump is the existing 880 against 1880 of gravity: 0.936 s
//    airborne, 206 px at the apex. The crouched rush box tops out 106 px up,
//    which his own box clears from 0.068 s to 0.869 s into a jump -- a 0.80 s
//    clear window. The rush crosses him in 0.224 s and first touches 0.374 s
//    after it starts, so with "JUMP!" shown 0.23 s before the rush the cue
//    lands 0.604 s before contact. Jumping the INSTANT the cue appears works
//    (clear until 0.869, danger over by 0.828) and so does jumping up to
//    0.536 s later. That window is the point: it is wide enough for an
//    ordinary reaction and does not demand a frame-perfect one.
//
//    Landing. He is back on the ground 0.936 s after he jumps, and RECOVERY
//    does not open until 1.37 s after the cue, so he always lands first.
//
//    Throwing. 0.15 s of windup plus about 0.21 s of stick flight is 0.36 s
//    from pressing F to the stick arriving, against a 1.50 s window -- so F
//    has to be down within about 1.14 s of "HIT NOW!".
// ---------------------------------------------------------------------------
const double EVE_GUARD_TIME     = 1.40;   // open stance, sticks bounce off
const double EVE_WARN_TIME      = 0.70;   // "WATCH OUT!" then "JUMP!"
const double EVE_WARN_TIME_FAST = 0.60;   // once rattled -- see EVE_RAGE_AFTER
const double EVE_JUMP_CUE_AT    = 0.23;   // "JUMP!" with this much warning left
const double EVE_RECOVER_TIME   = 1.50;   // the only window a stick counts in

// Past this many hits some cycles use the hurried warning instead.
const int    EVE_RAGE_AFTER     = 3;
const int    EVE_RAGE_PERCENT   = 45;

// The aggressor. He is drawn from primitives -- a red-shirt figure, as the
// concept preview used -- because no single standalone attacker sprite exists
// and the group artwork cannot be cut into one: the boys in it overlap each
// other, so any rectangle that takes one takes pieces of his neighbours.
const double EVE_RUSHER_HOME_X  = 590.0;  // his spot, in front of the group
const double EVE_RUSHER_TURN_X  = 250.0;  // how far past the player he carries

// He is drawn at a CONSTANT width, taken from the artwork's own aspect, and
// only his height drops when he charges -- so the change reads as a crouch
// rather than as the character shrinking. 66 px is not arbitrary: it is what
// keeps the top of his box at 106, which is what the existing jump clears.
const double EVE_RUSHER_H       = 116.0;  // standing
const double EVE_RUSHER_LOW_H   =  66.0;  // crouched and charging
const double EVE_RUSH_SPEED     = 520.0;
const double EVE_RETURN_SPEED   = 700.0;  // harmless, and quick enough to be open

// The rush box is pulled well inside the drawing: no invisible reach.
const double EVE_RUSH_BOX_W     = 0.68;
const double EVE_RUSH_BOX_H     = 0.88;

const int    EVE_RUSH_DAMAGE    = 40;
const double EVE_FEEDBACK_TIME  = 0.60;   // BLOCKED! / DODGED! / +1 HIT

// ---------------------------------------------------------------------------
//  Set-piece 4  --  the thief
//
//  He runs in on the twelve run_frames and stops dead in front. THE CHARACTER
//  is the one who goes down on his knees: beg.png is him, not the thief, and
//  boy_01_with_fx / boy_02 are his slap. Six slaps and the thief is gone.
//
//  The three heights below are all the CHARACTER'S poses, so they are sized
//  against the walk frames rather than against the thief. character_N.png is
//  drawn PLAYER_BASE_H tall but only fills 524 of its 554-pixel canvas, so a
//  standing pose cut down to its own content wants about 163 to match.
// ---------------------------------------------------------------------------
const int    L03_RUN_FRAME_COUNT = 12;
const double L03_RUN_ASPECT      = 256.0 / 355.0;   // run_frames are all 256x355
const int    CHOR_SLAPS_TO_CLEAR = 6;
const double CHOR_H          = 200.0;
const double CHOR_START_X    = 1300.0;
const double CHOR_STOP_X     = 448.0;   // where he pulls up: inside slap reach
const double CHOR_SPEED      = 430.0;
const double CHOR_FRAME_TIME = 0.055;
const double CHOR_FLEE_SPEED = 620.0;

// ---------------------------------------------------------------------------
//  The thief does not just stand there any more
//
//      OUT OF REACH -> APPROACH -> SLAP OPPORTUNITY -> RETREAT -> OUT OF REACH
//
//  He waits a little beyond arm's length, darts in, gives the player about
//  nine tenths of a second, and backs off again. A slap thrown at the wrong
//  moment swings at air.
//
//  THE REACH, measured rather than guessed:
//
//    The player's body box is 360.0 .. 411.1 and his slap adds
//    CHOR_SLAP_REACH beyond it, so his hand covers 411.1 .. 506.1. The thief's
//    hit box is his drawn rect inset by 22%/56%, which at CHOR_STOP_X (448)
//    lands 479.7 .. 560.5 -- 26 px inside the hand, so a slap connects. Out at
//    CHOR_FAR_X (600) it starts at 631.7, a clear 126 px past the hand, so the
//    same slap cannot possibly reach him.
//
//    Hits are resolved at the END of the windup, not when H goes down, so what
//    matters is where he is when the hand actually lands.
// ---------------------------------------------------------------------------
const double CHOR_FAR_X          = 600.0;   // out of reach, waiting
const double CHOR_APPROACH_SPEED = 340.0;   // 152 px in 0.45 s
const double CHOR_RETREAT_SPEED  = 430.0;   // and back out in 0.35 s
const double CHOR_WAIT_TIME      = 0.75;    // the beat before he comes in
const double CHOR_OPEN_TIME      = 0.90;    // how long the opening stays up
const double CHOR_HIT_PAUSE      = 0.35;    // reeling, before he backs off
const double CHOR_MISS_RECOVER   = 0.40;    // after swinging at nothing
const double CHOR_SLAP_REACH     = 95.0;    // how far past his body the hand goes
const double CHOR_CUE_TIME       = 0.55;    // "TOO FAR!" / "HIT!"

const double BEG_H           = 128.0;   // beg.png        -- him down on his knees
const double SLAP_H          = 168.0;   // boy_01 / boy_02 -- him back on his feet
const double SLAP_SHOW_TIME  = 0.30;    // how long the thief flinches from a hit

// One slap, in two frames: the hand lands, then he follows through. Reusing
// the walk cycle's frame timer would tie the slap to his footfalls, so it has
// its own two durations -- and only these two.
const double SLAP_UP_TIME    = 0.18;    // boy_01_with_fx.png -- up, and the hit
const double SLAP_HIT_TIME   = 0.26;    // boy_02.png         -- the follow-through

// The walk frames keep their whole canvas and the pose sprites are cut down to
// their content, so a raw swap would drop his feet by this much. Lifting the
// cut-down poses by it keeps him standing on the same line through the swap.
const double L03_POSE_FOOT_LIFT = 4.0;

// ---------------------------------------------------------------------------
//  Set-piece 5  --  the traffic jam, and fire from above
//
//  A long line of parked vehicles he jumps up onto and CROSSES. This is the
//  last stretch of the road, so it is walked like the rest of the level: he
//  drives it himself, the backdrop and the parked line scroll past him as he
//  pushes forward, and reaching the last backdrop is the end of Level 03.
//
//  It used to run on a sixty-second clock with the line sliding past on its
//  own and his progress frozen -- which meant the street never changed, going
//  forward achieved nothing, and the level ended on a timer rather than on
//  arriving anywhere. The crossing is a distance now, not a wait.
// ---------------------------------------------------------------------------
const int    JAM_VEHICLE_COUNT = 18;        // how many are laid end to end
const double JAM_VEHICLE_H     = 132.0;
const double JAM_BASE_Y        =  40.0;
// The line his feet stand on, up on the roofs.
//
// This used to be JAM_VEHICLE_H - 14, which put his feet INSIDE the vehicles:
// measured off a frame, the roofs come out at y 166-170 while his shoes landed
// at 165, so every bus and CNG rose past his ankles and the whole row read as
// being in front of him even though it is drawn behind him. Standing him on
// top of the roofs rather than in them is what fixes that -- the draw order
// was already right.
const double JAM_ROOF_Y        = JAM_BASE_Y + JAM_VEHICLE_H - 3.0;

// His dodging room on the roofs, and where pushing forward stops moving HIM
// and starts moving the WORLD.
//
// Walking right he crosses the screen until he reaches L03_JAM_MAX_X; past
// that the backdrop, the parked line and the falling fire all slide left
// underneath him instead, which is what makes the cars travel backwards
// relative to him. Walking left he simply gives ground, down to
// L03_JAM_MIN_X -- the road never runs backwards.
//
// The gap between the two is his dodge window: 440 px, nearly four flame
// widths, so stepping aside is a real option and not a pixel-perfect one.
const double L03_JAM_MIN_X = 120.0;
const double L03_JAM_MAX_X = 560.0;
const double L03_JAM_MOVE_SPEED = 250.0;

// The fire, and it has to be read against the crossing it falls on.
//
// At one flame every 0.45-1.05 s, dropping at 430 px/s -- barely a second of
// warning each -- and all aimed within 230 px of wherever he was standing,
// that was not a dodge, it was a shower, and it emptied a full 600 bar in
// four seconds.
//
// One every 0.9-1.8 s is a steady rain of them with roughly two seconds of
// warning apiece, and now that walking forward slides them past him he has
// two ways out of each one instead of one.
const int    MAX_FLAMES        = 6;
const double FLAME_H           = 130.0;
const double FLAME_FALL_SPEED  = 300.0;
const double FLAME_MIN_GAP     = 0.90;
const double FLAME_MAX_GAP     = 1.80;

// ---------------------------------------------------------------------------
//  Coins  --  the only thing that scores on this level
//
//  Level 03 had no coins at all: gScore sat at zero from Kunipara to AUST and
//  the HUD's score and coin read-outs never moved. These put the same scoring
//  on this level that the other two already have, through the same shared
//  collectCoin() -- so there is still exactly one place in the game where a
//  coin is worth COIN_VALUE, and the save system banks this level's total the
//  way it banks everyone else's.
//
//  They ride the road: they drift past on the walking stretches, hold still
//  while a set-piece has him stopped, and slide with the parked line while he
//  crosses the jam. Some sit at head height, because a coin he has to jump for
//  is worth more than a coin he walks into -- and the jump apex is 206 px, so
//  L03_COIN_HIGH stays well inside it.
// ---------------------------------------------------------------------------
const int    MAX_L03_COINS    = 8;
const double L03_COIN_SIZE    = 52.0;
const double L03_COIN_MIN_GAP = 1.10;     // seconds between drops, at a walk
const double L03_COIN_MAX_GAP = 2.30;
const double L03_COIN_LOW     = 16.0;     // above whatever he is standing on
const double L03_COIN_HIGH    = 150.0;
const double L03_COIN_POP     = 0.70;     // how long the "+10" floats

// ---------------------------------------------------------------------------
//  The arrival  --  AUST
//
//  The jam is the last thing between him and the gate, not the end of the
//  level. Crossing it drops him back onto the pavement in front of
//  bg_last.jpeg and he runs the last stretch in on his own; the win is
//  declared when he gets there.
//
//  bg_last.jpeg is 4:3 and the window is 16:9, so it is drawn to COVER the
//  window at its true aspect rather than stretched to fit it -- a 33%
//  horizontal stretch on a photograph of a real building is very obvious.
//  The overflow runs off the top and bottom and OpenGL clips it.
//
//  L03_GATE_BG_BIAS says which part of that overflow to keep: 1.0 pins the
//  top of the picture to the top of the screen, 0.0 pins the bottom. 0.78
//  keeps the university sign on screen while leaving him steps to stand on.
// ---------------------------------------------------------------------------
const double L03_GATE_START_X   = -140.0;   // off the left edge, running in
const double L03_GATE_STOP_X    =  520.0;   // where he stops, under the sign
const double L03_GATE_RUN_SPEED =  300.0;   // faster than his road pace
const double L03_GATE_BG_BIAS   =    0.78;

// Flames land anywhere across the street, not just inside his dodge window:
// the ones ahead of him are what make pushing forward a decision.
const double FLAME_SPAWN_MIN_X =  40.0;
const double FLAME_SPAWN_MAX_X = 1080.0;

#endif // CONFIG_HPP
