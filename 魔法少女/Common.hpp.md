```
 # pragma once
# include <Siv3D.hpp>

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
	Rect n_rect{ m_rect.x,m_rect.y - 25,200,45 };

	bool m_visible;
};

class Button
{
public:

	Button(const Vec2& centerPos, const String& text, bool visible = true)
		: m_visible{ visible }
		, m_text{ text }
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

		if (m_rect.mouseOver())
		{
			m_rect.rounded(20).draw(HSV{ Palette::Gray,0.5 });
		}

	}

	void draw(ChoiceColor ch)const
	{


		m_rect.rounded(20).draw(HSV{ ch.baseColor });
		m_rect.stretched(-3).rounded(20).drawFrame(3, ch.frameColor);

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

	if (json.isObject())
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

```
