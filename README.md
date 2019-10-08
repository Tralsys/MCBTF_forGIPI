# MCBTF_forGIPI
実物のマスコンやブレーキの接点を活用してGIPIに情報を送信するためのスケッチ

あくまでサンプルスケッチです。

将来的に、GUIを用いて各種設定を行えるようにするソフトウェアを開発し、それを用いて楽にこれ同様のスケッチを作成することができるようにする予定です。

## 設定解説
|行|名称|解説|
|-|-|-|
|[12](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L12)|`READMODE`|`pinMode`に設定する値を定義します。<br/>外部プルダウン抵抗を使用する場合は、`INPUT`を設定します。<br/>内部プルアップ抵抗を使用する場合は、`INPUT_PULLUP`を設定します。|
|[24](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L24)|`MaxP`|力行段数を設定します。P5まである場合、「5」を設定します。|
|[25](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L25)|`MaxB`|制動段数を設定します。B8まである場合、非常ブレーキ段を含めて「9」を設定します。|
|[27](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L27)|`PPinNum`|力行ハンドルにおいて、接点状態の組み合わせからハンドル位置を指定する場合、接点につないだピンの個数をここに指定します。|
|[28](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L28)|`PPin`|力行ハンドルの接点をつないだ端子を指定します。`PPinNum`で指定した個数だけ、設定してください。|
|[29](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L29)|`PHL`|力行ハンドルの接点状態とハンドル位置の紐づけを行います。<br/>接点が接触していれば`H`を、離れていれば`L`を、`PPin`で指定したピンの順に、P0から指定してください。|
|[38](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L38)|`BPinNum`|制動ハンドルにおいて、接点状態の組み合わせからハンドル位置を指定する場合、接点につないだピンの個数をここに指定します。|
|[39](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L39)|`BPin`|制動ハンドルの接点をつないだ端子を指定します。`BPinNum`で指定した個数だけ、設定してください。|
|[40](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L40)|`BHL`|制動ハンドルの接点状態とハンドル位置の紐づけを行います。<br/>接点が接触していれば`H`を、離れていれば`L`を、`BPin`で指定したピンの順に、B0から指定してください。|
|[47](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L47)|`RPinNum`|逆転ハンドルにおいて、接点状態の組み合わせからハンドル位置を指定する場合、接点につないだピンの個数をここに指定します。|
|[48](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L48)|`RPin`|逆転ハンドルの接点をつないだ端子を指定します。`PPinNum`で指定した個数だけ、設定してください。|
|[49](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L49)|`RHL`|逆転ハンドルの接点状態とハンドル位置の紐づけを行います。<br/>接点が接触していれば`H`を、離れていれば`L`を、`RPin`で指定したピンの順に、前, 中, 後の順で指定してください。|
|[56](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L56)|`TSMCMode`|TSマスコン互換のコマンドを送出するかどうかを設定します。<br/>`true`でTSMC互換コマンドを、`false`でGIPI規格コマンドを送出します。|
|[58](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L58)|`BVRPin`|制動ハンドル位置を、可変抵抗を用いて検出する場合、可変抵抗を接続したピンをここに指定します。<br/>不要時は`-1`を設定してください。|
|[59](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L59)|`PVRPin`|力行ハンドル位置を、可変抵抗を用いて検出する場合、可変抵抗を接続したピンをここに指定します。<br/>不要時は`-1`を設定してください。|
|[62](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L24)|`BVRNum`|analogRead値と制動ハンドル位置の紐づけを行う。<br/>B0から順に、数値が大きくなるように設定する。|

設定は以上です。

## 実装解説
### グローバル変数
|行|名称|解説|
|-|-|-|
|[67](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L67)|`BVRSiki`|制動ハンドル用可変抵抗の、位置判定の閾値を格納します。<br/>[86~88行目](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L86-L88)にて設定されます。|
|[123](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L123)|`OldPi`|前回周回時の力行ハンドル位置を記録します。|
|[124](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L124)|`OldBi`|前回周回時の制動ハンドル位置を記録します。|
|[125](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L125)|`OldRi`|前回周回時の逆転ハンドル位置を記録します。|

### 関数一覧
|行|名称|解説|
|-|-|-|
|[68-121](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L68-L121)|[`setup`](#void_setup)|起動時に1回だけ実行される。初期設定をここで行う。|
|[126-210](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L126-L210)|[`loop`](#void_loop)|`setup`完了後、無限に実行される。|
|[212-253](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L212-L253)|[`brcom`](#void_brcom)|制動ハンドル位置設定コマンドを送出します。|
|[254-283](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L254-L283)|[`nocom`](#void_nocom)|力行ハンドル位置設定コマンドを送出します。|
|[285-311](/MCBTF_forGIPI/MCBTF_forGIPI.ino#L285-L311)|[`aveB`](#void_aveB)|可変抵抗を用いた制動ハンドルの位置検出を行う。|

### <a name="void_setup">void setup(void)</a>
Now writing...

### <a name="void_loop">void loop(void)</a>
Now writing...

### <a name="void_brcom">void brcom(int command)</a>
Now writing...

### <a name="void_nocom">void nocom(int command)</a>
Now writing...

### <a name="void_aveB">void aveB(float num)</a>
Now writing...

### 処理の流れ
Now printing...
