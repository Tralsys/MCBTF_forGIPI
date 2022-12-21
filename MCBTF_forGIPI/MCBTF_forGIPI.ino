/*
 Name:		MCBTF_forGIPI.ino
 Author:	Tetsu Otter
 Version:	2.0.0
*/
/*
このスケッチは、接点の開閉を読んでGIPIに情報を送信するためのものです。
*/
#define READ_MODE INPUT_PULLUP //PULLUPにするかどうか。

#if READ_MODE==INPUT_PULLUP
#define H LOW
#define L HIGH
#else
#define READ_MODE INPUT
#define H HIGH
#define L LOW
#endif // READ_MODE==INPUT_PULLUP

#pragma region 設定スタート
const int PowerMaxPositionNum = 4;
const int PPinCount = 4;//マスコン(力行ハンドル)の使用するピンの個数
const int PPin[PPinCount] = { 2,3,4,5 };//マスコン(力行ハンドル)の使用するピンの番号 不要な場合は先頭にマイナス値を入れる
const int PHL[PowerMaxPositionNum + 2][PPinCount] = 
{//上から"P0のときの接点状態", "P1のときの接点状態",,,を、中カッコ内にカンマ区切りでPPinに入力した順に入れる。接点が接触していればH、離れていればLとなる。
{ L,L,L,L },//中カッコのあとの「,」を忘れないように。
{ H,L,L,L },
{ H,H,L,L },
{ H,H,H,L },
{ H,H,H,H },
NULL
};//マスコンの接点と段数の関係

const int BrakeMaxPositionNum = 2;
const int BPinCount = 5;//ブレーキ(制動ハンドル)の使用するピンの個数
const int BPin[BPinCount] = { 7,8,9,10,11 };//ブレーキ(制動ハンドル)の使用するピンの番号 不要な場合は先頭にマイナス値を入れる
const int BHL[BrakeMaxPositionNum + 2][BPinCount] =
{//上から"B0のときの接点状態", "B1のときの接点状態",,,を、中カッコ内にカンマ区切りでBPinに入力した順に入れる。接点が接触していればH、離れていればLとなる。
{ H,H,H,H,H },//中カッコのあとの「,」を忘れないように。
{ L,L,L,L,L },
{ H,L,H,L,H },
NULL
};//ブレーキの接点と段数の関係

const int RPinCount = 2;//レバーサー(逆転ハンドル)の使用するピンの個数
const int RPin[RPinCount] = { 12,13 };//レバーサー(逆転ハンドル)の使用するピンの番号 不要な場合は先頭にマイナス値を入れる
const int RHL[4][RPinCount] =
{//接点が接触していればH、離れていればLとなる。
{ H,H },//レバーサー「前」位置の時のピン状態
{ L,L },//「中」位置の時のピン状態
{ H,L },//「後」位置の時のピン状態
NULL
};//レバーサーの接点と位置の関係

const bool TSMCMode = false;//TSマスコン互換コマンド送信オプション

#pragma endregion 設定終了

#define COMMAND_DELIMITER ("\r")

const int REVERSER_FORWARD = 0;
const int REVERSER_NEUTRAL = 1;
const int REVERSER_BACKWARD = 2;

int lastBrakeHandlePosition = BrakeMaxPositionNum;
int lastPowerHandlePosition = PowerMaxPositionNum;
int lastReverserHandlePosition = REVERSER_NEUTRAL;

bool isBrakeVRMode = false;
bool isPowerVRMode = false;
bool isReverserVRMode = false;

#pragma region Functino Declaration
void SetUpPin(const int* pinList, int length);

void printBrakeCommand(int command);
void printPowerCommand(int command);
void printReverserCommand(int command);

const int *getBrakePinSettingList(int handlePosition);
const int *getPowerPinSettingList(int handlePosition);
const int *getReverserPinSettingList(int handlePosition);

void checkAndPrintHandlePosition(
	bool isVRMode,
	const int* pinNumberList,
	const int *(*pinSettingList)(int),
	int pinCount,
	int maxHandlePosition,
	int *lastHandlePosition,
	void (*printCommand)(int)
);

bool isCurrentHandlePositionHere(const int *pinNumberList, const int *pinSettingList, int pinCount);
bool getCurrentHandlePosition(const int *pinNumberList, const int *(*pinSettingList)(int), int pinCount, int maxHandlePosition, int *result);

void SetPowerCommand(int pos, char* buf);
void SetBrakeCommand(int pos, char* buf);
void SetReverserCommand(int pos, char* buf);
#pragma endregion Functino Declaration

void SetUpPin(const int* pinList, int length)
{
	for (int i = 0; i < length; i++)
	{
		pinMode(pinList[i], READ_MODE);
	}
}

void setup() {
	Serial.begin(9600);
	while (!Serial);

	SetUpPin(PPin, PPinCount);
	SetUpPin(BPin, BPinCount);
	SetUpPin(RPin, 3);

	printBrakeCommand(BrakeMaxPositionNum);
	printPowerCommand(0);

	printReverserCommand(REVERSER_NEUTRAL);
}

void loop() {
	checkAndPrintHandlePosition(isBrakeVRMode, BPin, getBrakePinSettingList, BPinCount, BrakeMaxPositionNum, &lastBrakeHandlePosition, printBrakeCommand);
	checkAndPrintHandlePosition(isPowerVRMode, PPin, getPowerPinSettingList, PPinCount, PowerMaxPositionNum, &lastPowerHandlePosition, printPowerCommand);
	checkAndPrintHandlePosition(isReverserVRMode, RPin, getReverserPinSettingList, RPinCount, 2, &lastReverserHandlePosition, printReverserCommand);

	delay(5);
}

void printBrakeCommand(int num) {
	char cmd[16] = "";
	SetBrakeCommand(num, cmd);
	Serial.print(strcat(cmd, COMMAND_DELIMITER));
}
void printPowerCommand(int num) {
	char cmd[16] = "";
	SetPowerCommand(num, cmd);
	Serial.print(strcat(cmd, COMMAND_DELIMITER));
}
void printReverserCommand(int num) {
	char cmd[8] = "";
	SetReverserCommand(num, cmd);
	Serial.print(strcat(cmd, COMMAND_DELIMITER));
}

const int *getPowerPinSettingList(int handlePosition)
{
	return PHL[handlePosition];
}
const int *getBrakePinSettingList(int handlePosition)
{
	return BHL[handlePosition];
}
const int *getReverserPinSettingList(int handlePosition)
{
	return RHL[handlePosition];
}

void checkAndPrintHandlePosition(
	bool isVRMode,
	const int* pinNumberList,
	const int *(*pinSettingList)(int),
	int pinCount,
	int maxHandlePosition,
	int *lastHandlePosition,
	void (*printCommand)(int)
)
{
	int currentPosition;

	if (isVRMode)
	{
	}
	else if (!getCurrentHandlePosition(pinNumberList, pinSettingList, pinCount, maxHandlePosition, &currentPosition))
	{
		return;
	}
	
	if (*lastHandlePosition == currentPosition)
	{
		return;
	}
	
	printCommand(currentPosition);

	*lastHandlePosition = currentPosition;
}

bool isCurrentHandlePositionHere(const int *pinNumberList, const int *pinSettingList, const int pinCount)
{
	for (int i = 0; i < pinCount; i++)
	{
		if (digitalRead(pinNumberList[i]) != pinSettingList[i])
		{
			return false;
		}
	}

	return true;
}

bool getCurrentHandlePosition(const int *pinNumberList, const int *(*pinSettingList)(int), int pinCount, int maxHandlePosition, int *result)
{
	if (pinNumberList[0] < 0)
	{
		return false;
	}

	for (int i = 0; i <= maxHandlePosition; i++)
	{
		if (isCurrentHandlePositionHere(pinNumberList, pinSettingList(i), pinCount))
		{
			*result = i;
			return true;
		}
	}

	return false;
}

#pragma region SetCommand

const int TSMCPowerCommandListMaxIndex = 5;
const char TSMCPowerCommandList[TSMCPowerCommandListMaxIndex + 1][6] = {
	"TSA50",
	"TSA55",
	"TSA65",
	"TSA75",
	"TSA85",
	"TSA95",
};

const int TSMCBrakeCommandListMaxIndex = 9;
const char TSMCBrakeCommandList[TSMCBrakeCommandListMaxIndex + 1][6] = {
	"TSA50",
	"TSA45",
	"TSA35",
	"TSA25",
	"TSA15",
	"TSA05",
	"TSE99",
	"TSB40",
	"TSB30",
	"TSB20",
};

const int ReverserCommandListMaxIndex = 2;
const char TSMCReverserCommandList[ReverserCommandListMaxIndex + 1][6] = {
	"TSG99",
	"TSG50",
	"TSG00",
};

const char BIDSReverserCommandList[ReverserCommandListMaxIndex + 1][5] = {
	"TORF",
	"TORN",
	"TORB",
};

void SetPowerCommand(int pos, char* buf)
{
	if (!TSMCMode)
	{
		strcpy(buf, "TOP");
		itoa(pos, buf + 3, 10);
		return;
	}

	const char* cmd = pos <= 0
		? TSMCPowerCommandList[0]
		: TSMCPowerCommandList[min(pos, TSMCPowerCommandListMaxIndex)];

	strcpy(buf, cmd);
}

void SetBrakeCommand(int pos, char* buf)
{
	if (!TSMCMode)
	{
		strcpy(buf, "TOB");
		itoa(pos, buf + 3, 10);
		return;
	}

	const char* cmd = pos <= 0
		? TSMCBrakeCommandList[0]
		: TSMCBrakeCommandList[min(pos, TSMCBrakeCommandListMaxIndex)];

	strcpy(buf, cmd);
}

#define reverserCommandList(index) (TSMCMode ? TSMCReverserCommandList[index] : BIDSReverserCommandList[index])
void SetReverserCommand(int pos, char* buf)
{
	const char* cmd = pos <= 0
		? reverserCommandList(0)
		: reverserCommandList(min(pos, ReverserCommandListMaxIndex));

	strcpy(buf, cmd);
	return;
}

#pragma endregion SetCommand
