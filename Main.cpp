# include <Siv3D.hpp> // Siv3D v0.6.16
struct AIGene {
	double avoidWeight = 3.0;
	double sideWeight = 1.5;
	double attackWeight = 0.8;
	double snipeRate = 0.5;
	double detectDist = 110.0;


	AIGene mutate() const {
		AIGene next = *this;
		next.avoidWeight = Clamp(next.avoidWeight + Random(-0.5, 0.5), 0.5, 8.0);
		next.sideWeight = Clamp(next.sideWeight + Random(-0.4, 0.4), 0.0, 5.0);
		next.attackWeight = Clamp(next.attackWeight + Random(-0.3, 0.3), 0.1, 4.0);
		next.snipeRate = Clamp(next.snipeRate + Random(-0.15, 0.15), 0.0, 1.0);
		next.detectDist = Clamp(next.detectDist + Random(-15.0, 15.0), 60.0, 180.0);

		return next;
	}
};
struct weapon {
	double shotTimer=0.0;
	double interval=0.1;
	bool ifcanshot() {
		shotTimer -= Scene::DeltaTime();
		if(shotTimer<= 0) {
			shotTimer = interval;
			return true;
			
		}
		else {
			return false;
		}

	}
};
struct Bullet {
	Vec2 pos;
	Vec2 vel;
	double radious = 5.0;
	ColorF color = Palette::White;
	void move() {
		pos+=vel * Scene::DeltaTime();
	}
	void draw() const {
		Circle{ pos, radious }.draw(color);              
		Circle{ pos, radious * 0.5 }.draw(Palette::White); 
	}
};
struct Enemy {
	Vec2 pos;
	int hp = 3;
	double speed = 220;
	Vec2 moveDir{ 0.0,0.25 };
	double moveTimer = 1.5;
	double hitRadious;
	double shotTimer;
	Vec2 vel{ 0,0 };
	weapon mainWeapon{ 0.0,0.25 };
	AIGene gene;
	void move(const Array<Bullet>&playerBullets,Vec2 playerPos) {
		Vec2 dir{ 0,0 };
		for (const auto& pb : playerBullets) {
			double dist = pos.distanceFrom(pb.pos);
			if (dist< gene.detectDist) {
				Vec2 away = (pos - pb.pos).normalized();
				Vec2 side = Vec2{ -pb.vel.y, pb.vel.x }.normalized();
				if (away.dot(side) < 0 || (away.dot(side) == 0 && RandomBool())) {
					if (pos.x < 60.0 && side.x < 0) {
							side *= -1.0;
						}
						else if (pos.x > 740.0 && side.x > 0) {
							side *= -1.0;
						}
						else if (away.dot(side) < 0) {
							side *= -1.0;
						}
				}
				double power = (gene.detectDist - dist) / gene.detectDist;
				dir += (away*gene.avoidWeight + side * gene.sideWeight) * power * 3.0;
			}
		}
			double diffX = playerPos.x - pos.x;
			if (Abs(diffX) > 10.0) {
				dir.x += (diffX > 0 ? 1.0 : -1.0) * gene.attackWeight;
			}
			if (pos.x < 120.0) dir.x += ((160-pos.x)/160)*6.0;
			if (pos.x > 680.0) dir.x -= ((pos.x-640)/640)*6.0;
			if (pos.y < 80.0)  dir.y += 1.5;
			if (pos.y > 260.0) dir.y -= 1.5;
	if (!dir.isZero()) {
		dir.normalize();
	}
	
		vel = dir * speed;
		pos += dir * speed * Scene::DeltaTime();

		pos.x = Clamp(pos.x, 60.0, 740.0);
		pos.y = Clamp(pos.y, 40.0, 560.0);

	}
	void shoot(Array<Bullet>& bullets, Vec2 targetPos, Vec2 targetVel) {
		if (mainWeapon.ifcanshot()) {
			double bulletSpeed = 300.0;
			Vec2 bulletVel;
			
			if(RandomBool(gene.snipeRate) ){
				double dist = pos.distanceFrom(targetPos);
				double time = dist / bulletSpeed;
				Vec2 futurePos = targetPos + targetVel * time;
				futurePos.x = Clamp(futurePos.x, 40.0, 760.0);
				bulletVel = (futurePos - pos).normalized() * bulletSpeed;
			}
			else if (RandomBool(0.5)) {
				bulletVel = (targetPos - pos).normalized() * bulletSpeed;
			}else {
				bulletVel = Vec2{ 0, bulletSpeed };
			}

			bullets.push_back(Bullet{ pos, bulletVel, 6.0, Palette::Orangered });
		}
	}
	void draw() const {
		Circle{ pos, 40 }.draw(Palette::Seagreen);
	}
};

struct player {
	double speed = 220;
	Vec2 vel;
	Vec2 pos{ 400, 500 };
	double hitRadious = 8.0;
	weapon mainWeapon{ 0.0,0.25 };
	int zanki=3;
	bool isAI = true;
	AIGene gene;
	void move(const Array<Bullet>& enemyBullets, const Array<Enemy>& enemies) {
		Vec2 dir{ 0, 0 };
		if (!isAI) {
			if (KeyUp.pressed()) {

				dir.y -= 1;
			}
			if (KeyLeft.pressed()) {
				dir.x -= 1;
			}
			if (KeyDown.pressed()) {
				dir.y += 1;
			}
			if (KeyRight.pressed()) {
				dir.x += 1;
			}

			if (!dir.isZero()) {
				dir.normalize();
			}

			
		}
		else {
			for (const auto& eb : enemyBullets) {
				double dist = pos.distanceFrom(eb.pos);
				if (dist < gene.detectDist) {
					Vec2 away = (pos - eb.pos).normalized();  
					Vec2 side = Vec2{ -eb.vel.y, eb.vel.x }.normalized();
					if (away.dot(side) < 0||(away.dot(side)==0&&RandomBool())) {
						if (pos.x < 60.0 && side.x < 0) {
							side *= -1.0;
						}
						else if (pos.x > 740.0 && side.x > 0) {
							side *= -1.0;
						}
						else if (away.dot(side) < 0) {
							side *= -1.0;
						}
					}
					double power = (gene.detectDist - dist) / gene.detectDist;
					dir += (away * gene.avoidWeight + side * gene.sideWeight) * power * 3.0;
				}
			}
			if (!enemies.isEmpty()) {
				double diffX = enemies[0].pos.x - pos.x;
				if (Abs(diffX) > 10.0) {
					dir.x += (diffX > 0 ? 1.0 : -1.0) * gene.attackWeight;
				}
			}
			if (pos.x < 120.0) dir.x += ((160 - pos.x) / 160) * 6.0;;
			if (pos.x > 680.0) dir.x -= ((pos.x - 640) / 640) * 6.0;
			if (pos.y < 340.0)  dir.y += 1.5;
			if (pos.y > 520.0) dir.y -= 1.5;
		}
		    if (!dir.isZero()) {
			      dir.normalize();
		    }
			if (KeyShift.pressed()) {
				vel = dir * speed;
				pos += dir * (speed / 2) * Scene::DeltaTime();
			}
			else {
				vel = dir * speed;
				pos += dir * speed * Scene::DeltaTime();
			}

			pos.x = Clamp(pos.x, 60.0, 740.0);
			pos.y = Clamp(pos.y, 40.0, 560.0);
		
	}
	void shoot(Array<Bullet>& bullets, const Array<Enemy>& enemies) {
		if ((isAI||KeyZ.pressed())&& mainWeapon.ifcanshot()){
			double bulletSpeed = 300.0;
			Vec2 bulletVel{0,-bulletSpeed};
			if (!enemies.isEmpty()) {
				Vec2 targetPos = enemies[0].pos;
				Vec2 targetVel = enemies[0].vel;
				
				
				if(RandomBool(gene.snipeRate)) {
					double dist = pos.distanceFrom(targetPos);
					double time = dist / bulletSpeed;
					Vec2 futurePos = targetPos + targetVel * time;
					futurePos.x = Clamp(futurePos.x, 40.0, 760.0);
					bulletVel = (futurePos - pos).normalized() * bulletSpeed;
				}
			else if (RandomBool(0.5) ){
				bulletVel = (targetPos - pos).normalized() * bulletSpeed;
			}else {
				bulletVel = Vec2{ 0, -bulletSpeed };
			}
			}
			bullets.push_back(Bullet{ pos, bulletVel, 6.0, Palette::Orangered });
		}
	}
	void draw() const {
		Circle{ pos, 40 }.draw(Palette::Royalblue);
		//範囲がずれてるのはわざとだから変えるな
		Circle{ pos, hitRadious + 4.0 }.draw(Palette::White);
		Circle{ pos, hitRadious + 4.0 }.drawFrame(2.0, Palette::Red);
	}
};



void Main()
{
	AIGene buleGene;
	AIGene greenGene = buleGene.mutate();
	int generation = 1;
	int blueWins = 0;
	int greenWins = 0;

	player p;
	p.gene = buleGene;
	Array<Bullet> bullets;
	Array<Bullet> Enemybullets;
	Array<Enemy> enemies;
	Enemy initialEnemy{ Vec2{ 400, 150 }, 3 };
	initialEnemy.gene = greenGene;
	enemies.push_back(initialEnemy);
	int simSpeed = 1;
	while (System::Update()) {
		if (Key1.down())simSpeed = 1;
		if (Key2.down())simSpeed = 10;
		if (Key3.down())simSpeed = 50;
		if (Key4.down())simSpeed = 500;
		if (Key5.down())simSpeed = 5000;
		if (Key6.down())simSpeed = 40000;
		for (int step = 0; step < simSpeed; step++) {
			if (p.zanki > 0 && !enemies.isEmpty()) {
				bullets.remove_if([](const Bullet& b) {
					return (b.pos.y < -20.0 || b.pos.x < -20 || b.pos.x>820);
				});
				Enemybullets.remove_if([](const Bullet& en) {
					return (en.pos.y > 620.0||en.pos.x<-20||en.pos.x>820);
				});
				enemies.remove_if([](const Enemy& e) {
					return (e.hp <= 0);
				});

				p.move(Enemybullets, enemies);
				p.shoot(bullets, enemies);

				for (auto& b : bullets) {
					b.move();
				}
				for (auto& b : bullets) {
					for (auto& e : enemies) {
						if (Circle{ b.pos, 5 }.intersects(Circle{ e.pos, 8 })) {
							e.hp -= 1;
							b.pos.y = -100;
						}
					}
				}
				for (auto& eb : Enemybullets) {
					if (Circle{ eb.pos, 5 }.intersects(Circle{ p.pos, p.hitRadious })) {
						p.zanki -= 1;
						eb.pos.y = 1000;
					}
				}
				for (auto& e : enemies) {
					e.move(bullets, p.pos);
					e.shoot(Enemybullets, p.pos, p.vel);
				}
				for (auto& eb : Enemybullets) {
					eb.move();
				}
			}
			if (p.zanki <= 0) {
				generation += 1;
				greenWins += 1;
				buleGene = greenGene.mutate();
				p = player{};
				p.gene = buleGene;
				bullets.clear();
				Enemybullets.clear();
				enemies.clear();
				Enemy nextEnemy{ Vec2{ 400,150 },3 };
				nextEnemy.gene = greenGene;
				enemies.push_back(nextEnemy);
			}
			if (enemies.isEmpty()) {
				generation += 1;
				blueWins += 1;
				greenGene = buleGene.mutate();
				p = player{};
				p.gene = buleGene;
				bullets.clear();
				Enemybullets.clear();
				enemies.clear();
				Enemy nextEnemy{ Vec2{ 400,150 },3 };
				nextEnemy.gene = greenGene;
				enemies.push_back(nextEnemy);
			}
		}
			p.draw();
			for (auto& b : bullets) {
				b.draw();
			}
			for (auto& e : enemies) {
				e.draw();
			}
			for (auto& eb : Enemybullets) {
				eb.draw();
			}
			ClearPrint();
			Print << U"=== 第 " << generation << U" 世代 ===";
			Print << U"青: " << blueWins << U"勝 (HP:" << p.zanki << U") | 緑: " << greenWins << U"勝 (HP:" << (enemies.isEmpty() ? 0 : enemies[0].hp) << U")";
			Print << U"青の性格 -> 回避:" << p.gene.avoidWeight << U" 横:" << p.gene.sideWeight << U" 攻撃:" << p.gene.attackWeight << U" 偏差率:" << p.gene.snipeRate;
			if (!enemies.isEmpty()) {
				Print << U"緑の性格 -> 回避:" << enemies[0].gene.avoidWeight << U" 横:" << enemies[0].gene.sideWeight << U" 攻撃:" << enemies[0].gene.attackWeight << U" 偏差率:" << enemies[0].gene.snipeRate;
			}
				

			
		}
	}
