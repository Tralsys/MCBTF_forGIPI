/*
 Name:		MCBTF_forGIPI.ino
 Created:	2018/08/07 18:13:43
 Author:	Tetsu Otter
*/
/*
このスケッチは、接点の開閉を読んでGIPIに情報を送信するためのものです。
可変抵抗を用いて位置を検出するタイプのものは別にあります。
そのうちボタン操作やワンハンドルにも対応できるようにします。
*/
#define H HIGH
#define L LOW
const int PPinNum = 5;//マスコン(力行ハンドル)の使用するピンの個数
const int MaxP = 2;//力行段数
const int PPin[PPinNum] = { 2,3,4,5,6 };//マスコン(力行ハンドル)の使用するピンの番号
const bool PHL[MaxP + 1][PPinNum] = 
{//上から"P0のときの接点状態", "P1のときの接点状態",,,を、中カッコ内にカンマ区切りでPPinに入力した順に入れる。接点が接触していればH、離れていればLとなる。
{H,H,H,H,H},//中カッコのあとの「,」を忘れないように。
{L,L,L,L,L}
};//マスコンの接点と段数の関係

const int BPinNum = 5;//ブレーキ(制動ハンドル)の使用するピンの個数
const int MaxB = 2;//制動段数
const int BPin[BPinNum] = { 7,8,9,10,11 };//ブレーキ(制動ハンドル)の使用するピンの番号
const int BHL[MaxB + 1][BPinNum] =
{//上から"B0のときの接点状態", "B1のときの接点状態",,,を、中カッコ内にカンマ区切りでBPinに入力した順に入れる。接点が接触していればH、離れていればLとなる。
{ H,H,H,H,H },//中カッコのあとの「,」を忘れないように。
{ L,L,L,L,L }
};//ブレーキの接点と段数の関係

const int RPinNum = 2;//レバーサー(逆転ハンドル)の使用するピンの個数
const int RPin[RPinNum] = { 12,13 };//レバーサー(逆転ハンドル)の使用するピンの番号
const int RHL[3][RPinNum] =
{//接点が接触していればH、離れていればLとなる。
{ H,H },//レバーサー「前」位置の時のピン状態
{ L,L },//「中」位置の時のピン状態
{ H,L }//「後」位置の時のピン状態
};//レバーサーの接点と位置の関係


void setup() {
	Serial.begin(9600);
	while (!Serial);
	Serial.print("TOB" + String(MaxB) + "\r");
	Serial.print("TOP0\r");
	Serial.print("TORN\r");
}

int OldPi = 0;
int OldBi = MaxB;
int OldRi = 1;
void loop() {
	bool PCompTF = false;
	bool BCompTF = false;
	bool RCompTF = false;
	for (int i = 0; i < max(MaxB, MaxP); i++) {
		if (i < MaxP && PCompTF == false) {
			for (int pdr = 0; pdr < PPinNum; pdr++) {
				if (digitalRead(PPin[pdr]) != PHL[i][pdr]) {
					goto Pout;
				}
			}
			//P段数「i」と判断された。
			if (i != OldPi) {
				Serial.print("TOP" + String(i) + "\r");
				OldPi = i;
			}
			PCompTF = true;
		Pout:;
		}
		if (i < MaxB && BCompTF == false) {
			for (int bdr = 0; bdr < BPinNum; bdr++) {
				if (digitalRead(BPin[bdr]) != BHL[i][bdr]) {
					goto Bout;
				}
			}
			//B段数「i」と判断された。
			if (i != OldBi) {
				Serial.print("TOB" + String(i) + "\r");
				OldBi = i;
			}
			BCompTF = true;
		Bout:;
		}
		if (i < 3 && RCompTF == false) {
			for (int bdr = 0; bdr < BPinNum; bdr++) {
				if (digitalRead(BPin[bdr]) != BHL[i][bdr]) {
					goto Rout;
				}
			}
			//R位置「i」と判断された。
			if (OldRi != i) {
				switch (i)
				{
				case 0:
					Serial.print("TORF\r");
					break;
				case 1:
					Serial.print("TORN\r");
					break;
				case 2:
					Serial.print("TORB\r");
					break;
				}
				OldRi = i;
			}
			BCompTF = true;
		Rout:;
		}
		delay(2);
	}
	delay(5);
}
