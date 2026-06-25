# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Common.hpp"


void Main()
{
	
	Array<NovelScene> novelTimeline = LoadScenario(U"example/json/魔法少女.json");

	size_t sceneIndex = 0;//現在のシーン番号
	size_t commandIndex = 0; // そのシーン内でのコマンド（演出）番号

	String currentBG = U"なし";
	String currentBGM = U"なし";
	String currentSE = U"";
	String currentChara = U"なし";

	String currentSpeaker = U"";
	String currentText = U"";

	//現在再生中のBGM
	String currentPlaying = U"";

	// 選択肢の時間差表示用タイマー
	Stopwatch choiceTimer;
	const double CHOICE_DELAY_SEC = 1.0; // 選択肢が表示されるまでの待ち時間（秒）

	// パラメーター（好感度など）の管理
	HashTable<String, int32> variables;
	variables[U"aoi_love"] = 50;
	
	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{0});
	Window::Resize(900, 600);

	TextureAsset::Register({ U"BG1",{U"bg"} }, U"example/texture/壊れた街.png");

	TextureAsset::Register({ U"mascot",{U"character"} }, U"example/character/bird.png");

	TextureAsset::Register({ U"normal",{U"character"} }, U"example/character/majo_normal.png");
	TextureAsset::Register({ U"smile",{U"character"} }, U"example/character/majo_smile.png");
	TextureAsset::Register({ U"happy",{U"character"} }, U"example/character/majo_happy.png");
	TextureAsset::Register({ U"cry",{U"character"} }, U"example/character/majo_cry.png");
	TextureAsset::Register({ U"madness",{U"character"} }, U"example/character/majo_madness.png");
	TextureAsset::Register({ U"hollow",{U"character"} }, U"example/character/majo_hollow.png");
	TextureAsset::Register({ U"black",{U"character"} }, U"example/character/majo_black.png");

	AudioAsset::Register({ U"BGM1", { U"BGM" } }, Audio::Stream, U"example/BGM/Violence_of_Kawaii.mp3");
	AudioAsset::Register({ U"BGM2", { U"BGM" } }, Audio::Stream, U"example/BGM/ナイト・パレード.mp3");
	AudioAsset::Register({ U"BGM3", { U"BGM" } }, Audio::Stream, U"example/BGM/In_a_stormy_night.mp3");

	AudioAsset::Register({ U"SE1", { U"SE" } }, Audio::Stream, U"example/.SE/決定6.mp3");
	AudioAsset::Register({ U"SE2", { U"SE" } }, Audio::Stream, U"example/.SE/パンチで壁を破壊.mp3");
	AudioAsset::Register({ U"SE3", { U"SE" } }, Audio::Stream, U"example/.SE/打撃7.mp3");
	AudioAsset::Register({ U"SE4", { U"SE" } }, Audio::Stream, U"example/.SE/status02.mp3");

	MessageBox message{ Rect{20, 440, 860, 140},true };

	Button menu{ Rect{ 800,20,80 },true };
	const Texture t2{ 0xF035C_icon, 80 };

	Button choice{ Vec2{ 0, 0 }, U"" };
	Button choice2{ Vec2{ 0, 0 }, U"" };
	Button Lastchoice{ Vec2{ 0, 0 }, U"" };

	while (System::Update())
	{
		if (not currentBG.isEmpty())
		{
			TextureAsset(currentBG).scaled(0.8).draw();


		}
		
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

			if (currentLove < 30)		nextEndScene = U"scene_violence";
			else if (currentLove < 80)	nextEndScene = U"scene_07_LastAct_N";
			else                        nextEndScene = U"scene_happiness";

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
				else if (cmd.type == U"se")         currentSE = cmd.value;
				else if (cmd.type == U"chara_show")  currentChara = cmd.value;
				else if (cmd.type == U"chara_hide")  currentChara = U"なし";

				commandIndex++;
			}
		}


		TextureAsset(currentChara).drawAt(250, 250);
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
						choice.setText(currentScene.choices[0].text, Vec2{ 450, 200 });
						choice2.setText(currentScene.choices[1].text, Vec2{ 450, 300 });

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
					// 条件を満たしたら即座に次のシーンへ遷移させる
					sceneIndex = FindSceneIndex(novelTimeline, currentScene.defaultNextScene);
					commandIndex = 0;
					choiceTimer.reset();
					currentSpeaker = U"";
					currentText = U"";

					// 次のフレームを待たずに、新シーンの処理（背景やBGM変更）を走らせるために continue
					continue;
				}
				else
				{
					ClearPrint();
					Print << U"\n【シナリオ終了、またはルート分岐待ち】";
				}
			}
		}

		if (not currentSE.isEmpty())
		{
			// 0.5秒かけてフェードアウト
			AudioAsset(currentSE).play();

			currentSE.clear();

		}

		if (currentBGM != currentPlaying)
		{
			// 前の曲が流れていたらフェードアウト
			if (not currentPlaying.isEmpty() && AudioAsset::IsRegistered(currentPlaying))
			{
				// 0.5秒かけてフェードアウト
				AudioAsset(currentPlaying).stop(0.5s);
			}

			// 新しい曲をフェードイン再生
			if (AudioAsset::IsRegistered(currentBGM))
			{
				// 0.5秒かけてフェードイン（ループ再生）
				AudioAsset(currentBGM).play(); 
			}

			// 現在再生中の状態を更新
			currentPlaying = currentBGM;

		}
		
	}
}

