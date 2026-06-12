# include <Siv3D.hpp> // OpenSiv3D v0.6.11

struct Textset
{
	String name;
	String text;
	//String image;
	//String bgm;
	//String se;
	String type;
	String test;
};

struct Choice
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

	void draw(Choice ch)const
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

Array<Choice> choices =
{
	{Palette::Darkred,Palette::Orangered,Palette::White},//暴力
	{Palette::Hotpink,Palette::Mediumvioletred,Palette::White},//魔法
	{Palette::Black,Palette::Gray,Palette::White},//最後の審判

};

void Main()
{
	TextReader reader{ U"テストテキスト.txt" };

	size_t index = 0;
	size_t min_index = 0;
	Array<Textset> testS;
	String line;
	
	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });
	Window::Resize(900, 600);

	while (reader.readLine(line))
	{
		const Array<String> items = line.split(U' ');



		testS << Textset{ items[0],items[1],items[2],items[3]};
	}

	TextureAsset::Register({ U"majo",{U"Texture"} }, U"example/character/majo_girl_majokko.png");
	TextureAsset::Register({ U"mascot",{U"Texture"} }, U"example/character/bird_maruitori.png");
	MessageBox test{ Rect{20, 440, 860, 140},true };
	Button menu{ Rect{ 800,20,80 },true };
	Button choice{ Vec2{300,300},testS[testS.size() - 1].test};
	Button choice2{ Vec2{600,300},testS[testS.size() - 1].test };
	Button Lastchoice{ Vec2{Scene::Center()},testS[testS.size() - 1].test};

	const Texture t2{ 0xF035C_icon, 80 };

	

	while (System::Update())
	{
		


		TextureAsset(U"majo").scaled(0.8).drawAt(250, 350);
		TextureAsset(U"mascot").scaled(0.5).drawAt(650, 300);

		

		if(menu.clicked())
		{
			ClearPrint();
			Print(U"セーブロードとExitと会話ログとか出したい");
		}

		

		if (!menu.mouseOvered() && Cursor::OnClientRect() && MouseL.down())	index++;

		index = Clamp(index, min_index,static_cast<size_t>(testS.size()-1));
		
		

		test.draw();
		test.setText(testS[index].text, testS[index].name);
		menu.draw(t2);
		menu.updateCursorStyle();

		if(testS[index].type == U"choice")
		{
			Rect{ Scene::Size() }.draw(HSV{Palette::Gray,0.5 });
			choice.draw(choices[0]);
			choice.updateCursorStyle();

			choice2.draw(choices[1]);
			choice2.updateCursorStyle();

			if (choice.clicked() || choice2.clicked())
			{
				
				index = 0;
			}
		}
		//Lastchoice.draw(choices[2]);
	}
}

