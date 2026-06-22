# include <Siv3D.hpp> // OpenSiv3D v0.6.11

struct Command
{
    String type;        // "text"(セリフ), "bg"(背景変更), "chara"(立ち絵変更), "sound"(効果音) など
    String name;        // 話者の名前（typeが"text"の時のみ使用）
    String value;       // 背景ファイル名、セリフ本文、効果音名など、タイプに応じた文字列
};

struct Choice
{
    String text;        // 選択肢のテキスト
    String nextScene;   // 選んだときの次のシーン名
    String targetParam; // 加算したいパラメーター名（例: "heroine_likability"）
    int32 value;        // 加算する値（例: 10）
};

struct NovelScene
{
    String sceneId;          // シーンの識別子（例: "scene_01_intro"）
    Array<Command> commands; // 上から順番に実行する演出・セリフのリスト
    Array<Choice> choices;   // コマンドがすべて終わった後に表示する選択肢（なければ空）
    String defaultNextScene; // 選択肢がない場合、自動的に遷移する次のシーン名（ED分岐前なら空）
};


struct ChoiceColor
{
	ColorF baseColor;
	ColorF frameColor;
	ColorF TextColor;
};

class MessageBox
{
public:

	MessageBox(const Rect& rect, bool visible)
		: m_rect{ rect }
		, m_visible{ visible }
	{

		// 特に何もしない

	}

	void setText(const String& text, const String& text1)
	{
		m_text = text;
		n_text = text1;
	}

	void draw() const
	{
		m_rect.rounded(60).draw(HSV{ Palette::White, 0.8 }).drawFrame(2, 8, ColorF{ 0.5 });
		m_font(m_text).draw(25, m_rect.stretched(-22), ColorF{ 0.1, 0.1, 0.1 });

		n_rect.rounded(30).draw(HSV{ Palette::White }).drawFrame(2, 5, ColorF{ 0.5 });
		m_font(n_text).draw(23, n_rect.stretched(-6), ColorF{ Palette::Black });
	}

private:

	Font m_font = Font{ 25 };

	String m_text;
	String n_text;

	Rect m_rect;
	Rect n_rect{m_rect.x,m_rect.y - 25,200,45};
	
	bool m_visible;
};

class Button
{
public:

	Button(const Vec2& centerPos, const String& text, bool visible = true)
		: m_visible{ visible }
		,m_text{ text }
	{
		// 文字を描画したときのサイズ（RectF）を取得
		// ※ drawAt を使う想定なので、基準点を中心にするための regionAt を使用
		const RectF textRegion = m_font(text).regionAt(centerPos);

		// 左右と上下の余白（パディング）を設定
		const double paddingX = 30.0;
		const double paddingY = 15.0;

		// 文字のサイズに余白を足して、Rect に変換して保持
		m_rect = textRegion.stretched(paddingX, paddingY).asRect();
	}

	Button(const Rect& rect, bool visible)
		: m_rect{ rect }
		, m_visible{ visible }
	{

		// 特に何もしない

	}

	void setText(const String& text, const Vec2& centerPos)
	{
		m_text = text;

		// 文字を描画したときのサイズ（RectF）を取得
		const RectF textRegion = m_font(text).regionAt(centerPos);

		// 左右と上下の余白（コンストラクタと同じ設定）
		const double paddingX = 30.0;
		const double paddingY = 15.0;

		// 新しいサイズで m_rect を更新
		m_rect = textRegion.stretched(paddingX, paddingY).asRect();
	}

	void updateCursorStyle() const
	{
		if (!m_visible)
		{
			return;
		}

		if (m_rect.mouseOver())
		{
			Cursor::RequestStyle(CursorStyle::Hand);
		}
	}

	bool clicked() const
	{
		if (!m_visible)
		{
			return false;
		}

		return m_rect.leftClicked();
	}

	bool mouseOvered()
	{
		if (!m_visible)
		{
			return false;
		}

		return m_rect.mouseOver();

	}

	void draw(Texture m_tex) const
	{
		m_rect.rounded(20).draw(HSV{ Palette::White });
		m_rect.stretched(-3).rounded(20).drawFrame(3, ColorF{ 0.5 });

		m_tex.drawAt(m_rect.center().x, m_rect.center().y, ColorF{ 0.25 });

		if(m_rect.mouseOver())
		{
			m_rect.rounded(20).draw(HSV{ Palette::Gray,0.5 });
		}
	
	}

	void draw(ChoiceColor ch)const
	{
		

		m_rect.rounded(20).draw(HSV{ ch.baseColor });
		m_rect.stretched(-3).rounded(20).drawFrame(3, ch.frameColor );

		if (m_rect.mouseOver())
		{
			HSV c{ ch.baseColor };
			c.v *= 0.3;
			m_rect.rounded(20).draw(HSV{ c,0.5 });
		}

		m_font(m_text).drawAt(m_rect.center(), ch.TextColor);
	}


private:

	Rect m_rect;

	bool m_visible;

	Font m_font = Font{ 25 };
	String m_text;
};

Array<ChoiceColor> choices =
{
	{Palette::Darkred,Palette::Orangered,Palette::White},//暴力
	{Palette::Hotpink,Palette::Mediumvioletred,Palette::White},//魔法
	{Palette::Black,Palette::Gray,Palette::White},//最後の審判

};

size_t FindSceneIndex(const Array<NovelScene>& timeline, const String& id)
{
	for (size_t i = 0; i < timeline.size(); ++i)
	{
		if (timeline[i].sceneId == id) return i;
	}
	return 0; // 見つからなかったらとりあえず最初へ
}

Array<NovelScene>LoadScenario(const String& path)
{
	Array<NovelScene> timeline;
	const JSON json = JSON::Load(path);
	if (not json) // もし読み込みに失敗したら
	{
		throw Error{ U"読み取りエラーです。確認してください。" };
	}

	if(json.isObject())
	{
		for (const auto& [sceneName, sceneData] : json[U"scene"])
		{
			NovelScene data;
			data.sceneId = sceneName;

			//次に進むシーン名があるか
			if (sceneData[U"defaultNextScene"].isString())
			{
			
				data.defaultNextScene = sceneData[U"defaultNextScene"].getString();
			}

			//commandsの中身が配列になってるか
			if (sceneData[U"commands"].isArray())
			{
				for (const auto& cmdElement : sceneData[U"commands"].arrayView())
				{
					Command cmd;//データの受け皿（一時的に保存する用）

					//必須データの読み込み
					if (cmdElement.contains(U"type")) cmd.type = cmdElement[U"type"].getString();
					if (cmdElement.contains(U"value")) cmd.value = cmdElement[U"value"].getString();

					//オプションデータの安全な読み込み
					if (cmdElement.contains(U"name") && cmdElement[U"name"].isString())
					{
						cmd.name = cmdElement[U"name"].getString();
					}
					else
					{
						cmd.name = U""; // キーがない場合は空文字にしておく
					}

					//リストへの追加
					data.commands.push_back(cmd);
				}
			}

			//選択肢があるか
			if (sceneData[U"choices"].isArray())
			{
				//選択肢を一つずつループで処理
				for (const auto& choiceElement : sceneData[U"choices"].arrayView())
				{
					Choice choice;//データの受け皿（一時的に保存する用）

					//テキストと次シーンの読み込み
					if (choiceElement.contains(U"text")) choice.text = choiceElement[U"text"].getString();
					if (choiceElement.contains(U"nextScene")) choice.nextScene = choiceElement[U"nextScene"].getString();

					//スコアやパラメータ関連
					//スコア関連は存在しないこともあるので contains で守る
					if (choiceElement.contains(U"targetParam") && choiceElement[U"targetParam"].isString())
					{
						//変動させたいパラメータ名があるか
						choice.targetParam = choiceElement[U"targetParam"].getString();
					}

					if (choiceElement.contains(U"value") && choiceElement[U"value"].isNumber())
					{
						//パラメータをどれだけ変化させるか
						choice.value = choiceElement[U"value"].get<int32>();
					}
					else
					{
						//データがなければ0を入れる
						choice.value = 0;
					}
					//完成した選択肢をリストに追加
					data.choices.push_back(choice);
				}
			}
			// タイムラインに追加
			timeline.push_back(data);

		}

	}
	return timeline;
}

void Main()
{
	
	Array<NovelScene> novelTimeline = LoadScenario(U"example/json/ノベルテスト.json");

	size_t sceneIndex = 0;//現在のシーン番号
	size_t commandIndex = 0; // そのシーン内でのコマンド（演出）番号

	String currentBG = U"なし";
	String currentBGM = U"なし";
	String currentChara = U"なし";

	String currentSpeaker = U"";
	String currentText = U"";

	//現在再生中のBGM
	String currentPlaying;

	// 選択肢の時間差表示用タイマー
	Stopwatch choiceTimer;
	const double CHOICE_DELAY_SEC = 1.5; // 選択肢が表示されるまでの待ち時間（秒）

	// パラメーター（好感度など）の管理
	HashTable<String, int32> variables;
	variables[U"aoi_love"] = 0;
	
	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });
	Window::Resize(900, 600);


	TextureAsset::Register({ U"majo",{U"Texture"} }, U"example/character/majo.png");
	TextureAsset::Register({ U"mascot",{U"Texture"} }, U"example/character/bird.png");

	TextureAsset::Register({ U"nomal",{U"Texture"} }, U"example/character/test.png");
	TextureAsset::Register({ U"blush",{U"Texture"} }, U"example/character/test_blush.png");
	TextureAsset::Register({ U"happy",{U"Texture"} }, U"example/character/test_happy.png");

	AudioAsset::Register(U"BGM1", Audio::Stream, U"example/test.mp3");

	MessageBox message{ Rect{20, 440, 860, 140},true };

	Button menu{ Rect{ 800,20,80 },true };
	const Texture t2{ 0xF035C_icon, 80 };

	Button choice{ Vec2{ 0, 0 }, U"" };
	Button choice2{ Vec2{ 0, 0 }, U"" };
	Button Lastchoice{ Vec2{ 0, 0 }, U"" };

	while (System::Update())
	{
		// 1. シナリオ終了チェック
		if (sceneIndex >= novelTimeline.size())
		{
			ClearPrint();
			Print << U"物語は終了しました。";
			continue;
		}

		const auto& currentScene = novelTimeline[sceneIndex];

		// 2. 自動分岐判定シーンの処理
		if (currentScene.sceneId == U"scene_check_end")
		{
			int32 currentLove = variables[U"aoi_love"];
			String nextEndScene = U"";

			if (currentLove >= 20)		nextEndScene = U"scene_true_end";
			else if (currentLove >= 10)	nextEndScene = U"scene_normal_end";
			else                        nextEndScene = U"scene_bad_end";

			sceneIndex = FindSceneIndex(novelTimeline, nextEndScene);
			commandIndex = 0;
			choiceTimer.reset();
			currentSpeaker = U"";
			currentText = U"";
			continue;
		}

		// 3. 演出コマンドの「自動連続実行」ロジック
		while (commandIndex < currentScene.commands.size())
		{
			const auto& cmd = currentScene.commands[commandIndex];

			if (cmd.type == U"text")
			{
				currentSpeaker = cmd.name;
				currentText = cmd.value;
				break;
			}
			else
			{
				if (cmd.type == U"bg")              currentBG = cmd.value;
				else if (cmd.type == U"bgm")         currentBGM = cmd.value;
				else if (cmd.type == U"chara_show")  currentChara = cmd.value;
				else if (cmd.type == U"chara_hide")  currentChara = U"なし";

				commandIndex++;
			}
		}


		TextureAsset(currentChara).scaled(0.5).drawAt(250, 350);
		TextureAsset(U"mascot").scaled(0.5).drawAt(650, 300);

		// デバッグ情報の表示
		/*ClearPrint();
		Print << U"【現在のシーン】: " << currentScene.sceneId;
		Print << U"好感度(aoi_love): " << variables[U"aoi_love"];*/

		message.draw();
		message.setText(currentText, currentSpeaker);

		menu.draw(t2);
		menu.updateCursorStyle();

		if(menu.clicked())
		{
			ClearPrint();
			Print(U"セーブロードとExitと会話ログとか出したい");
		}
			
		if (commandIndex < currentScene.commands.size())
		{
			// まだ次のセリフが残っている場合：画面クリック（メニュー外）で次へ
			if (!menu.mouseOvered() && Cursor::OnClientRect() && MouseL.down())
			{
				commandIndex++;
			}
		}
		else
		{
			// すべてのコマンド（最後のセリフ）の表示が完了している場合
			if (!currentScene.choices.isEmpty())
			{
				// タイマースタート
				if (!choiceTimer.isStarted())
				{
					choiceTimer.start();
				}

				// 一定時間経ったら選択肢ボタンを画面中央付近に縦に並べて表示
				if (choiceTimer.sF() >= CHOICE_DELAY_SEC)
				{
					// 背景を少し暗くして選択肢を目立たせる（オプション）
					Rect{ Scene::Size() }.draw(ColorF{ 0, 0, 0, 0.3 });

					if (currentScene.choices.size() >= 2)
					{
						choice.setText(currentScene.choices[0].text, Vec2{ 300, 300 });
						choice2.setText(currentScene.choices[1].text, Vec2{ 600, 300 });

						choice.draw(choices[0]);
						choice.updateCursorStyle();

						choice2.draw(choices[1]);
						choice2.updateCursorStyle();

						// 左のボタンがクリックされたとき
						if (choice.clicked())
						{
							const auto& c = currentScene.choices[0];
							if (not c.targetParam.isEmpty()) variables[c.targetParam] += c.value; // パラメーター加算

							sceneIndex = FindSceneIndex(novelTimeline, c.nextScene); // シーン遷移
							commandIndex = 0;
							choiceTimer.reset();
							currentSpeaker = U""; currentText = U"";
						}

						// 右のボタンがクリックされたとき
						if (choice2.clicked())
						{
							const auto& c = currentScene.choices[1];
							if (not c.targetParam.isEmpty()) variables[c.targetParam] += c.value;

							sceneIndex = FindSceneIndex(novelTimeline, c.nextScene);
							commandIndex = 0;
							choiceTimer.reset();
							currentSpeaker = U""; currentText = U"";
						}
					}
					else if (currentScene.choices.size() == 1)
					{
						
						Lastchoice.setText(currentScene.choices[0].text, Scene::Center());

						Lastchoice.draw(choices[2]);
						Lastchoice.updateCursorStyle();

						if (Lastchoice.clicked())
						{
							const auto& c = currentScene.choices[0];
							if (not c.targetParam.isEmpty()) variables[c.targetParam] += c.value;

							sceneIndex = FindSceneIndex(novelTimeline, c.nextScene);
							commandIndex = 0;
							choiceTimer.reset();
							currentSpeaker = U""; currentText = U"";
						}
					}
					
				}
			}
			else
			{
				// 選択肢がない場合の自動遷移（「次のシーンへ」ボタンをメッセージボックスの上に配置）
				if (!currentScene.defaultNextScene.isEmpty())
				{
					if ((!menu.mouseOvered() && Cursor::OnClientRect() && MouseL.down()))
					{
						sceneIndex = FindSceneIndex(novelTimeline, currentScene.defaultNextScene);
						commandIndex = 0;
						choiceTimer.reset();
						currentSpeaker = U"";
						currentText = U"";
					}
				}
				else
				{
					Print << U"\n【シナリオ終了、またはルート分岐待ち】";
				}
			}
		}
		

		if (currentBGM != currentPlaying)
		{
			if (not currentPlaying.isEmpty())
			{
				// 0.5秒かけてフェードアウト
				AudioAsset(currentPlaying).stop(0.5s);
			}

			if (AudioAsset::IsRegistered(currentBGM)) // 事前に登録されているか確認
			{
				// 0.5秒かけてフェードイン再生（ループ再生）
				AudioAsset(currentBGM).play();
			}

			// 「現在再生中」の状態を更新
			currentPlaying = currentBGM;
		}
		
	}
}

