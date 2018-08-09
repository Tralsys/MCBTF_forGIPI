/*
 Name:		MCBTF_forGIPI.ino
 Created:	2018/08/07 18:13:43
 Author:	Tetsu Otter
*/
/*
このスケッチは、接点の開閉を読んでGIPIに情報を送信するためのものです。
ブレーキのみ、可変抵抗を用いた位置検出に対応(α版)
そのうちボタン操作やワンハンドルにも対応できるようにすると思います。
*/
#define H HIGH
#define L LOW

//設定スタート
const int MaxP = 2;//力行段数
const int MaxB = 2;//制動段数

const int PPinNum = 5;//マスコン(力行ハンドル)の使用するピンの個数
const int PPin[PPinNum] = { 2,3,4,5,6 };//マスコン(力行ハンドル)の使用するピンの番号 不要な場合は先頭にマイナス値を入れる
const bool PHL[MaxP + 1][PPinNum] = 
{//上から"P0のときの接点状態", "P1のときの接点状態",,,を、中カッコ内にカンマ区切りでPPinに入力した順に入れる。接点が接触していればH、離れていればLとなる。
{H,H,H,H,H},//中カッコのあとの「,」を忘れないように。
{L,L,L,L,L},
{H,L,H,L,H}
};//マスコンの接点と段数の関係

const int BPinNum = 5;//ブレーキ(制動ハンドル)の使用するピンの個数
const int BPin[BPinNum] = { 7,8,9,10,11 };//ブレーキ(制動ハンドル)の使用するピンの番号 不要な場合は先頭にマイナス値を入れる
const int BHL[MaxB + 1][BPinNum] =
{//上から"B0のときの接点状態", "B1のときの接点状態",,,を、中カッコ内にカンマ区切りでBPinに入力した順に入れる。接点が接触していればH、離れていればLとなる。
{ H,H,H,H,H },//中カッコのあとの「,」を忘れないように。
{ L,L,L,L,L },
{ H,L,H,L,H }
};//ブレーキの接点と段数の関係

const int RPinNum = 2;//レバーサー(逆転ハンドル)の使用するピンの個数
const int RPin[RPinNum] = { 12,13 };//レバーサー(逆転ハンドル)の使用するピンの番号 不要な場合は先頭にマイナス値を入れる
const int RHL[3][RPinNum] =
{//接点が接触していればH、離れていればLとなる。
{ H,H },//レバーサー「前」位置の時のピン状態
{ L,L },//「中」位置の時のピン状態
{ H,L }//「後」位置の時のピン状態
};//レバーサーの接点と位置の関係

const bool TSMCMode = false;//TSマスコン互換コマンド送信オプション

const int BVRPin = -1;//ブレーキ(制動ハンドル)を可変抵抗を用いて位置検出する際の、接続したアナログピン番号
const int PVRPin = -1;//マスコン(力行ハンドル)を可変抵抗を用いて位置検出する際の、接続したアナログピン番号
					  //不要時はマイナス値を入れる。

const int BVRNum[MaxB + 1] = { 10,100,1000 };//Analogピンの入力値一覧
											 //B0,B1,B2...の順に入れる B0のほうが数値が小さくなるように

//設定終了

float BVRSiki[MaxB];
void setup() {
	Serial.begin(9600);
	while (!Serial);
	brcom(MaxB);
	nocom(0);
	Serial.print("TORN\r");
	if (BPin[0] < 0 && BVRPin >= 0) {
		for (int i = 0; i < MaxB; i++) {
			BVRSiki[i] = (BVRNum[i] + BVRNum[i + 1]) / 2;
		}
	}
	if (BPin[0] < 0 && BVRPin < 0) {
		Serial.println("エラー : ブレーキのピンが有効化されていません。");
		while (true)
		{
			digitalWrite(13, HIGH);
			delay(500);
			digitalWrite(13, LOW);
			delay(200);
			digitalWrite(13, HIGH);
			delay(100);
			digitalWrite(13, LOW);
			delay(200);
			Serial.println("エラー : ブレーキのピンが有効化されていません。");
		}
	}
	else if(BPin[0] >= 0 && BVRPin >= 0)
	{
		Serial.println("エラー : ブレーキのピンが有効化されすぎています。");
		while (true)
		{
			digitalWrite(13, HIGH);
			delay(200);
			digitalWrite(13, LOW);
			delay(500);
			digitalWrite(13, HIGH);
			delay(200);
			digitalWrite(13, LOW);
			delay(500);
			Serial.println("エラー : ブレーキのピンが有効化されすぎています。");
		}
	}
}

int OldPi = 0;
int OldBi = MaxB;
int OldRi = 1;
void loop() {
	float BAve;
	bool PCompTF = false;
	bool BCompTF = false;
	bool RCompTF = false;
	for (int i = 0; i <= max(MaxB, MaxP); i++) {
		if (i <= MaxP && !PCompTF && i != OldPi && PPin[0] >= 0 && PVRPin < 0) {
			for (int pdr = 0; pdr < PPinNum; pdr++) {
				if (digitalRead(PPin[pdr]) != PHL[i][pdr]) {
					goto Pout;
				}
			}
			//P段数「i」と判断された。
			nocom(i);
			OldPi = i;
			PCompTF = true;
		Pout:;
		}
		if (i <= MaxB && !BCompTF && i!= OldBi && BPin[0] >= 0 && BVRPin < 0) {
			for (int bdr = 0; bdr < BPinNum; bdr++) {
				if (digitalRead(BPin[bdr]) != BHL[i][bdr]) {
					goto Bout;
				}
			}
			//B段数「i」と判断された。
			brcom(i);
			OldBi = i;
			BCompTF = true;
		Bout:;
		}
		if (i < 3 && !RCompTF && i != OldRi) {
			if (RPin[0] >= 0) {
				for (int bdr = 0; bdr < BPinNum; bdr++) {
					if (digitalRead(BPin[bdr]) != BHL[i][bdr]) {
						goto Rout;
					}
				}
				//R位置「i」と判断された。
				switch (i)
				{
				case 0:
					if (TSMCMode) {
						Serial.print("\r");
					}
					else
					{
						Serial.print("TORF\r");
					}
					break;
				case 1:
					if (TSMCMode) {
						Serial.print("\r");
					}
					else
					{
						Serial.print("TORN\r");
					}
					break;
				case 2:
					if (TSMCMode) {
						Serial.print("\r");
					}
					else
					{
						Serial.print("TORB\r");
					}
					break;
				}
				OldRi = i;
				BCompTF = true;
			Rout:;
			}
		}
		if (BPin[0] < 0 && BVRPin >= 0) {
			BAve += (int)analogRead(BVRPin);
			BAve += (int)analogRead(BVRPin);
		}
		delay(1);
	}
	if (BPin[0] < 0 && BVRPin >= 0) {
		BAve /= max(MaxB, MaxP);
		aveB(BAve);
	}
	delay(5);
}

void brcom(int command) {
	if (TSMCMode) {
		switch (command) {
		case 0:
			Serial.print("TSA50\r");
			break;
		case 1:
			Serial.print("TSA45\r");
			break;
		case 2:
			Serial.print("TSA35\r");
			break;
		case 3:
			Serial.print("TSA25\r");
			break;
		case 4:
			Serial.print("TSA15\r");
			break;
		case 5:
			Serial.print("TSA05\r");
			break;
		case 6:
			Serial.print("TSE99\r");
			break;
		case 7:
			Serial.print("TSB40\r");
			break;
		case 8:
			Serial.print("TSB30\r");
			break;
		case 9:
			Serial.print("TSB20\r");
			break;
		default:
			Serial.print("TSB20\r");
			break;
		}
	}
	else {
		Serial.print("TOB" + String(command) + "\r");
	}
}
void nocom(int command) {
	if (TSMCMode) {
		switch (command) {
		case 0:
			Serial.print("TSA50\r");
			break;
		case 1:
			Serial.print("TSA55\r");
			break;
		case 2:
			Serial.print("TSA65\r");
			break;
		case 3:
			Serial.print("TSA75\r");
			break;
		case 4:
			Serial.print("TSA85\r");
			break;
		case 5:
			Serial.print("TSA95\r");
			break;
		default:
			Serial.print("TSA95\r");
			break;
		}
	}
	else {
		Serial.print("TOP" + String(command) + "\r");
	}
}

void aveB(float num) {
	if (num > BVRSiki[MaxB - 1]) {
		if (OldBi != MaxB) {
			brcom(MaxB);
			OldBi = MaxB;
		}
		goto aveBout;
	}
	if(num < BVRSiki[0])
	{
		if (OldBi != 0) {
			brcom(0);
			OldBi = 0;
		}
		goto aveBout;
	}
	if (BVRSiki[OldBi - 1] < num && num < BVRSiki[OldBi]) {
		goto aveBout;
	}
	for (int i = 1; i < MaxB - 1; i++) {
		if (BVRSiki[i - 1] < num && num < BVRSiki[i]) {
			brcom(i);
			goto aveBout;
		}
	}
aveBout:;
}