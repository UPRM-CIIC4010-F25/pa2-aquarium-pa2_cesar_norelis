#include "Aquarium.h"
#include <cstdlib>


string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        case AquariumCreatureType::Axolotl:
            return "Axolotl";
        case AquariumCreatureType::Jellyfish:
            return "Jellyfish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite)
{
    m_baseSpeed = static_cast<float>(speed);
    m_speed = m_baseSpeed;
    m_originalSpeed = m_baseSpeed;
    m_baseRadius = getCollisionRadius();
    m_normalSprite = sprite;
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();
    float deltaTime = ofGetLastFrameTime();
    updateBoost(deltaTime);
    updatePredator(deltaTime);
    reduceDamageDebounce();
    move();
}

void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::startBoost(PowerUpType type) {
    startBoost(5.0f, type);
}

void PlayerCreature::startBoost(float seconds, PowerUpType type) {
    m_boosted = true;
    m_hasActiveBoost = true;
    m_currentBoostType = type;
    m_boostTimer = seconds;

    auto scene = dynamic_cast<AquariumGameScene*>(ofGetAppPtr());

    if (type == PowerUpType::SPEED) {
        float intendedSpeed = m_speed * 1.1f;
        const float MAX_SPEED_CAP = m_baseSpeed * 1.5f;

        bool reachedCap = intendedSpeed >= MAX_SPEED_CAP;
        m_speed = reachedCap ? MAX_SPEED_CAP : intendedSpeed;

        if (scene) {
            const float ratio = m_speed / m_baseSpeed;
            if (ratio >= 1.5f) {
                scene->showBoostMessage("MAX SPEED BOOST");
            } else {
                scene->showBoostMessage("SPEED BOOST!");
            }
        }

        if (m_speed >= m_baseSpeed * 1.5f) {
            ofLogNotice() << "MAX SPEED BOOST";
        } else {
            ofLogNotice() << "Speed boost active: " << m_speed;
        }
    } else if (type == PowerUpType::SIZE) {
        m_sizeBoostMultiplier = 1.3f;
        float baseRadius = m_inPredatorMode ? m_predatorCollisionRadius : m_baseRadius;
        setCollisionRadius(baseRadius * m_sizeBoostMultiplier);
        if (scene) {
            scene->showBoostMessage("SIZE BOOST!");
        }
    }
}


void PlayerCreature::updateBoost(float deltaTime) {
    if (!m_hasActiveBoost) return;

    m_boostTimer -= deltaTime;
    if (m_boostTimer <= 0.0f) {
        if (m_currentBoostType == PowerUpType::SPEED) {
            m_speed = m_baseSpeed;
            ofLogNotice() << "Power-up expired. Speed reset to " << m_baseSpeed;
        } else if (m_currentBoostType == PowerUpType::SIZE) {
            m_sizeBoostMultiplier = 1.0f;
            float baseRadius = m_inPredatorMode ? m_predatorCollisionRadius : m_baseRadius;
            setCollisionRadius(baseRadius);
            ofLogNotice() << "Size boost expired. Hitbox reset.";
        }

        m_boosted = false;
        m_hasActiveBoost = false;
    }
}

void PlayerCreature::updatePredator(float deltaTime) {
    if (!m_inPredatorMode) {
        return;
    }

    m_predatorTimer = std::max(0.0f, m_predatorTimer - deltaTime);
    if (ofGetElapsedTimef() >= m_predatorEndTime) {
        ofLogNotice() << "Predator mode expired.";
        deactivatePredatorMode();
    }
}

void PlayerCreature::activatePredatorMode(float seconds, std::shared_ptr<GameSprite> predatorSprite) {
    m_inPredatorMode = true;
    m_predatorTimer = seconds;
    m_predatorEndTime = ofGetElapsedTimef() + seconds;

    if (!m_normalSprite) {
        m_normalSprite = m_sprite;
    }

    if (predatorSprite) {
        m_predatorSprite = predatorSprite;
        setSprite(m_predatorSprite);
    }

    setCollisionRadius(m_predatorCollisionRadius * m_sizeBoostMultiplier);
    ofLogNotice() << "Predator mode activated for " << seconds << " seconds.";
}

void PlayerCreature::deactivatePredatorMode() {
    m_inPredatorMode = false;
    m_predatorTimer = 0.0f;
    m_predatorEndTime = 0.0f;

    if (m_normalSprite) {
        setSprite(m_normalSprite);
    }

    setCollisionRadius(m_baseRadius * m_sizeBoostMultiplier);
}

void PlayerCreature::draw() const {
    
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce
    }
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
    ofSetColor(ofColor::white); // Reset color

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

class Axolotl : public NPCreature {
public:
    Axolotl(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
        : NPCreature(x, y, speed, sprite) {
        m_dx = 1;      
        m_dy = 0;      
        normalize();

        setCollisionRadius(40); 
        m_value = 3;   
        m_creatureType = AquariumCreatureType::Axolotl;
    }

    void move() override {
        m_x += m_dx * m_speed;

        if (m_x <= 0 || m_x >= m_width) {
            m_dx = -m_dx;
            if (m_sprite) m_sprite->setFlipped(m_dx < 0);
        }

    }

    void draw() const override {
        if (m_sprite) {
            m_sprite->draw(m_x, m_y);
        }
    }
};

class Jellyfish : public NPCreature {
public:
    Jellyfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
        : NPCreature(x, y, speed, sprite)
    {
        m_dx = 0;
        m_dy = 1;
        normalize();

        setCollisionRadius(30);
        m_value = 2;
        m_creatureType = AquariumCreatureType::Jellyfish;
    }

    void move() override {
        m_y += m_dy * m_speed;

        if (m_y <= 0 || m_y >= m_height) {
            m_dy = -m_dy;
        }
    }

    void draw() const override {
        if (m_sprite) {
            m_sprite->draw(m_x, m_y);
        }
    }
};

// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_axolotl = std::make_shared<GameSprite>("axolotl.png", 80, 50);
    this->m_jellyfish = std::make_shared<GameSprite>("jellyfish.png", 60, 80);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return this->m_big_fish->clone();
            
        case AquariumCreatureType::NPCreature:
            return this->m_npc_fish->clone();

        case AquariumCreatureType::Axolotl:
            return this->m_axolotl->clone();
        
        case AquariumCreatureType::Jellyfish:
            return this->m_jellyfish->clone();

        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    for (auto& creature : m_creatures) {
        creature->move();
    }
    this->Repopulate();
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
}


void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}



void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        case AquariumCreatureType::Axolotl:
            this->addCreature(std::make_shared<Axolotl>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Axolotl)));
            break;
        case AquariumCreatureType::Jellyfish:
            this->addCreature(std::make_shared<Jellyfish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Jellyfish)));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }

}

int Aquarium::getCurrentLevel() const {
    if (m_aquariumlevels.empty()) {
        return 0;
    }

    int maxLevelIndex = static_cast<int>(m_aquariumlevels.size() - 1);
    return std::min(currentLevel, maxLevelIndex);
}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

//  Imlementation of the AquariumScene
void AquariumGameScene::Update() {
    if (m_lastKnownLevel < 0 && m_aquarium) {
        m_lastKnownLevel = m_aquarium->getCurrentLevel();
    }

    this->m_player->update();

    float deltaTime = ofGetLastFrameTime();
    if (!m_activePowerUp) {
        float x = ofRandom(100, ofGetWidth() - 100);
        float y = ofRandom(100, ofGetHeight() - 100);

        m_activePowerUp = std::make_shared<PowerUp>(
            x, y, PowerUpType::SPEED,
            std::make_shared<GameSprite>("powerup.png", 40, 40)
        );

        m_powerUpLifeTimer = m_powerUpLifetime;
        ofLogNotice() << "Spawned a power-up!";
    } else {
        m_powerUpLifeTimer -= deltaTime;

        bool collected = checkCollision(m_player, m_activePowerUp);
        bool expired = m_powerUpLifeTimer <= 0.0f;

        if (collected) {
            m_player->startBoost(m_activePowerUp->getType());

            if (m_player->getSpeed() >= m_player->getBaseSpeed() * 1.5f) {
                showBoostMessage("MAX SPEED BOOST");
            } else {
                showBoostMessage("SPEED BOOST!");
            }

            ofLogNotice() << "Player collected Speed Boost!";
        } else if (expired) {
            ofLogNotice() << "Power-up expired!";
        }

        if (collected || expired) {
            m_activePowerUp.reset();
            m_powerUpLifeTimer = 0.0f;
        }
    }

    if (m_boostMessageTimer > 0.0f) {
        m_boostMessageTimer -= deltaTime;
        if (m_boostMessageTimer <= 0.0f) {
            m_boostMessage.clear();
        }
    }

    if (this->updateControl.tick()) {
        auto event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;
            if(event->creatureB != nullptr){
                event->print();
                auto npc = std::dynamic_pointer_cast<NPCreature>(event->creatureB);
                if(npc && npc->GetType() == AquariumCreatureType::Jellyfish){
                    ofLogNotice() << "A jellyfish sting harms the player!";
                    this->m_player->loseLife(3*60);
                    if(this->m_player->getLives() <= 0){
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                } else if(npc && npc->GetType() == AquariumCreatureType::Axolotl && this->m_player->isPredatorMode()){
                    ofLogNotice() << "Predator mode spares the axolotl.";
                } else {
                    bool predatorActive = this->m_player->isPredatorMode();
                    bool isAxolotl = npc && npc->GetType() == AquariumCreatureType::Axolotl;
                    bool canEat = predatorActive || isAxolotl || this->m_player->getPower() >= event->creatureB->getValue();
                    if(!canEat){
                        ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                        this->m_player->loseLife(3*60); // 3 frames debounce, 3 seconds at 60fps
                        if(this->m_player->getLives() <= 0){
                            this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                            return;
                        }
                    }
                    else{
                        this->m_aquarium->removeCreature(event->creatureB);
                        this->m_player->addToScore(1, event->creatureB->getValue());
                        if (this->m_player->getScore() % 25 == 0){
                            this->m_player->increasePower(1);
                            ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                        }
                        
                    }
                }
                
                

            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }

        this->m_aquarium->update();

        int currentLevel = this->m_aquarium->getCurrentLevel();
        if (currentLevel != m_lastKnownLevel) {
            auto spriteManager = this->m_aquarium->getSpriteManager();
            std::shared_ptr<GameSprite> predatorSprite = spriteManager ? spriteManager->GetSprite(AquariumCreatureType::BiggerFish) : nullptr;
            this->m_player->activatePredatorMode(10.0f, predatorSprite);
            showBoostMessage("PREDATOR MODE!");
            m_lastKnownLevel = currentLevel;
        }
    }
}

void AquariumGameScene::showBoostMessage(const std::string& msg) {
    m_boostMessage = msg;
    m_boostMessageTimer = 4.0f; // show message for longer visibility
}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    
    if (m_activePowerUp) {
    m_activePowerUp->draw();
}
    this->paintAquariumHUD();
    
  // Draw the boost message if active
if (!m_boostMessage.empty()) {
    ofSetColor(ofColor::yellow);
    ofDrawBitmapString(m_boostMessage, ofGetWidth() / 2 - 50, 100); // adjust position
    ofSetColor(ofColor::white); // reset color
    
}
}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}

std::vector<AquariumCreatureType> AquariumLevel::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for (const auto& node : m_levelPopulation) {
        if (!node) {
            continue;
        }

        int desiredPopulation = std::max(0, node->population);
        int delta = desiredPopulation - node->currentPopulation;
        if (delta <= 0) {
            continue;
        }

        ofLogVerbose() << "to Repopulate :  " << delta;
        toRepopulate.insert(toRepopulate.end(), static_cast<size_t>(delta), node->creatureType);
        node->currentPopulation += delta;
    }
    return toRepopulate;
}

bool Level_4::isCompleted() {
    return false;
}
