# Sandboxie Plus / Classic

<p align='center'>
<a href='./README.md'>EN</a> | <a href='./README_zh_CN.md'>中文</a> | 日本語
</p>

[![Plus license](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Classic license](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![GitHub Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![GitHub Pre-Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![GitHub Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![GitHub Codespell Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml) [![WinGet Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/winget.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/winget.yml) [![Gurubase](https://img.shields.io/badge/Gurubase-Ask%20Sandboxie%20Guru-006BFF)](https://gurubase.io/g/sandboxie)

[![Roadmap](https://img.shields.io/badge/Roadmap-Link%20-blue?style=for-the-badge)](https://www.wilderssecurity.com/threads/updated-sandboxie-plus-roadmap.456886/) [![Join our Discord Server](https://img.shields.io/badge/Join-Our%20Discord%20Server%20for%20bugs,%20feedback%20and%20more!-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

|  システム要件  |  リリースノート  |  コントリビューション ガイドライン  |  セキュリティ ポリシー  |  行動規範  |
|         :---:         |          :---:         |          :---:                |          :---:            |          :---:            |
| Windows 7 以降 (64 ビット) |  [CHANGELOG.md](./CHANGELOG.md)  |  [CONTRIBUTING.md](./CONTRIBUTING.md)  |   [SECURITY.md](./SECURITY.md)  |  [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md)  |

Sandboxie は、Windows NT ベースのオペレーティング システム向けのサンドボックスベースの分離ソフトウェアで、アプリケーションをローカル ドライブやマップ済みドライブ、Windows レジストリを恒久的に変更することなく実行またはインストールできる、安全な操作環境を作り出します。分離された仮想環境により、信頼できないプログラムのテストや Web 閲覧を制御された形で行えます。<br>

Sandboxie では、実質無制限のサンドボックスを作成し、それらを単独または同時に実行して、プログラムをホストおよび相互から分離できます。また、1 つのボックス内で好きなだけ多くのプログラムを同時に実行することもできます。

**注: これは Sandboxie のソースコード公開後に生まれたコミュニティ フォークであり、以前の開発の公式な継続ではありません（[プロジェクト履歴](#project-history) と [#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926) を参照してください）。**

## ⏬ ダウンロード

[最新リリース](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## ✨ 変更履歴

<a href='./CHANGELOG.md'>EN</a>

## 🚀 機能

Sandboxie には Plus と Classic の 2 つのエディションがあります。どちらも同じコア コンポーネントを共有しているため、安全性と互換性のレベルは同じです。
異なるのは、ユーザー インターフェイスで利用できる機能です。

Sandboxie Plus には、モダンな Qt ベースの UI があり、プロジェクトがオープンソース化されて以降に追加されたすべての新機能をサポートしています:

  * スナップショット マネージャー - 任意のボックスのコピーを取得し、必要に応じて復元できます
  * メンテナンス メニュー - 必要に応じて Sandboxie ドライバーとサービスをアンインストール/インストール/開始/停止できます
  * ポータブル モード - インストーラーを実行し、すべてのファイルをディレクトリに展開することを選択できます
  * プリント スプーラーやクリップボードなどの Windows コンポーネントへのアクセスをブロックする追加の UI オプション
  * Start/Run 制限とインターネット アクセス制限のための、より多くのカスタマイズ オプション
  * 不正アクセスからユーザー データを保護するプライバシー モード サンドボックス
  * システムコールとエンドポイントの可用性を制限するセキュリティ強化サンドボックス
  * すべてのボックス内プロセスを一時停止または終了するためのグローバル ホットキー
  * Windows Filtering Platform (WFP) をサポートするサンドボックスごとのネットワーク ファイアウォール
  * サンドボックスのリストはショートカット キー Ctrl+F で検索できます
  * グローバル設定とサンドボックス オプションの検索機能
  * サンドボックスを 7z ファイルへ/からインポート/エクスポートする機能
  * サンドボックスの Windows スタート メニューへの統合
  * サポートされていないブラウザー用のテンプレートを作成するブラウザー互換性ウィザード
  * Sandboxie Control (SbieCtrl) のグラフィカルな外観を再現するクラシック表示モード
  * ユーザーの問題を支援するトラブルシューティング ウィザード
  * 追加コンポーネントを介して機能を拡張または追加するアドオン マネージャー
  * スクリーンショットの取得防止を含む、ホストに対するサンドボックスの保護
  * サンドボックスが初期化、ボックス開始、終了、ファイル復元などのさまざまな段階を経るときにアクションを実行するトリガー システム
  * プロセスをサンドボックス化せず、その子プロセスをサンドボックス化する
  * プログラムにユーザー提供の SOCKS5 プロキシを自動的に使用させる
  * ブロックまたはリダイレクトによる DNS 制御
  * サンドボックス内の単一プロセスが占有できるメモリ量と、すべてのプロセスが占有できるメモリ総量を制限でき、ボックスごとのサンドボックス化プロセスの総数を制限できます
  * Sandboxie のオープンソース化前のバージョンとは完全に異なるトークン作成メカニズムにより、サンドボックスがシステム内でより独立します
  * 暗号化サンドボックス - AES ベースの信頼性の高いデータ ストレージ ソリューション
  * サンドボックス化されたプログラムが通常の方法で不要な一意の識別子を生成するのを防止
  * 構成済みまたは追加したい設定について、視覚的なヒントとツールチップでユーザーを支援する内部 INI エディター
  * システム既定のエディターに加えて外部テキスト エディターを構成する機能
  * 枠のアルファ透明度の制御
  * 権限を偽装したり、付与したり、昇格の試みをキャンセルできるカスタム UAC ダイアログ
  * モダンなアイコン。特定の場所では旧来のアイコンを使用することもできます
  * ユーザー インターフェイスのフォントを変更できます
  * サンドボックスまたはグループにカスタムの色やアイコンを使用できます

その他の機能は、[CHANGELOG.md](./CHANGELOG.md) ファイル内でショートカット キー Ctrl+F を使って記号 `=` を検索することで見つけることができます。

Sandboxie Classic には、もはや開発されていない MFC ベースの旧 UI があるため、Plus の機能に対するネイティブなインターフェイス サポートがありません。不足している機能の一部は Sandboxie.ini 構成ファイルで手動で構成したり、[カスタム スクリプト](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22) に置き換えたりできますが、最新のセキュリティ オプションを試したいユーザーには Classic エディションは推奨されません。

## 📚 ドキュメント

[Sandboxie ドキュメント](https://sandboxie-plus.github.io/sandboxie-docs) の GitHub コピーが現在も維持されていますが、新しい変更に合わせて更新し続けるには、より多くのボランティアが必要です。現在の問題を追跡するには、次のラベルも確認することをお勧めします: [Labels · sandboxie-plus/Sandboxie](https://github.com/sandboxie-plus/Sandboxie/labels)。

以前 Invincea によって維持されていた [旧 Sandboxie フォーラム](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) の部分的なアーカイブはまだ利用可能です。特定のものを探す必要がある場合は、次の検索クエリを使用できます: `site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/`。


## 🚀 Sandboxie の便利なツール

Sandboxie の機能は、次のような専門的なツールで強化できます:

  * [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - Sandboxie のトレース ログに詳細な出力を追加し、関連する Windows API 関数の呼び出しを一覧表示します
  * [SbieHide](https://github.com/VeroFess/SbieHide) - サンドボックス化されているアプリケーションから SbieDll.dll の存在を隠そうとします
  * [SandboxToys2](https://github.com/blap/SandboxToys2) - サンドボックス内のファイルとレジストリの変更を監視できます
  * [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - サンドボックス化されたプロセスに追加のユーザー モード制限を追加します
  * [WrapLocale](https://github.com/UserUnknownFactor/WrapLocale) - ネイティブの LangId 機能よりも柔軟なロケール偽装オプションを提供します

<a id="project-history"></a>
## 📌 プロジェクト履歴

|      タイムライン       |    メンテナー    |
|        :---         |       :---       |
| 2004 - 2013         | Ronen Tzur       |
| 2013 - 2017         | Invincea Inc.    |
| 2017 - 2020         | Sophos Group plc |
| 2020 年 4 月 8 日 - [オープンソース コード](https://community.sophos.com/sandboxie/f/forum/119641/important-sandboxie-open-source-code-is-available-for-download) | Sophos Ltd. |
| 2020 年 4 月 9 日以降 - プロジェクト フォーク | David Xanatos |

古い Sandboxie バージョンをお探しですか? [バージョン履歴](https://github.com/sandboxie-plus/sandboxie-old) をご覧ください。

現在の [ロードマップ](https://www.wilderssecurity.com/threads/updated-sandboxie-plus-roadmap.456886/) をご覧ください。

## 📌 プロジェクトのサポート / スポンサー

[<img align="left" height="64" width="64" src="./.github/images/binja-love.png">](https://binary.ninja/)
リバース エンジニアリングを支援するために [Binary Ninja](https://binary.ninja/) ライセンスを提供してくださった [Vector 35](https://vector35.com/) に感謝します。
<br>
Binary Ninja は、リバース エンジニア、マルウェア アナリスト、脆弱性研究者、ソフトウェア開発者向けの、マルチプラットフォームのインタラクティブな逆アセンブラー、デコンパイラー、バイナリ分析ツールです。<br>
<br>
[<img align="left" height="64" width="64" src="./.github/images/Icons8_logo.png">](https://icons8.de/)プロジェクトにアイコンを提供してくださった [Icons8](https://icons8.de/) に感謝します。
<br>
<br>
<br>

## 🤝 プロジェクトを支援する

Sandboxie が役に立つと思ったら、ぜひ当プロジェクトの [コントリビューション ガイドライン](./CONTRIBUTING.md) を通じて貢献してください。

## 📑 協力してくれたコントリビューター

- DavidBerdik - [Sandboxie Website Archive](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io) のメンテナー
- Jackenmen - Sandboxie の Chocolatey パッケージのメンテナー（[サポート](https://github.com/Jackenmen/choco-auto/issues?q=is%3Aissue+Sandboxie)）
- vedantmgoyal9 - Sandboxie の Winget Releaser のメンテナー（[サポート](https://github.com/vedantmgoyal9/winget-releaser/issues?q=is%3Aissue+Sandboxie)）
- blap - [SandboxToys2](https://github.com/blap/SandboxToys2) アドオンのメンテナー
- diversenok - セキュリティ分析と PoC / セキュリティ修正
- TechLord - Team-IRA / リバース エンジニアリング
- hg421 - セキュリティ分析と PoC / コードレビュー
- hx1997 - セキュリティ分析と PoC
- mpheath - Plus インストーラーの作者 / コード修正 / コラボレーター
- offhub - ドキュメント追加 / コード修正 / Qt5 パッチとビルド スクリプト / コラボレーター
- LumitoLuma - Qt5 パッチとビルド スクリプト
- QZLin - [sandboxie-docs](https://sandboxie-plus.github.io/sandboxie-docs/) テーマの作者
- isaak654 - テンプレート / ドキュメント / コード修正 / コラボレーター
- typpos - UI 追加 / ドキュメント / コード修正
- Yeyixiao - 機能追加
- Deezzir - 機能追加
- wzxjohn - コード修正、ドキュメント追加
- okrc - コード修正
- Sapour - コード修正
- lmou523 - コード修正
- sredna - Classic インストーラーのコード修正
- weihongx9315 - コード修正
- marti4d - コード修正
- jorgectf - CodeQL ワークフロー
- stephtr - CI / 認定
- yfdyh000 - Plus インストーラーのローカライズ対応
- Dyras - テンプレート追加
- cricri-pingouin - UI 修正
- Valinwolf - UI / アイコン
- daveout - UI / アイコン
- kokofixcomputers - [Discord](https://discord.gg/S4tFu6Enne) チャンネルのサポート メンバー
- NewKidOnTheBlock - 変更履歴の修正
- Naeemh1 - ドキュメント追加
- APMichael - テンプレート追加
- 1mm0rt41PC - ドキュメント追加
- Luro223 - ドキュメント追加
- lwcorp - ドキュメント追加
- wilders-soccerfan - ドキュメント追加
- LepordCat - ドキュメント追加
- stdedos - ドキュメント追加
- habatake - UI 追加、コード修正
- Polyester6719 - ドキュメント追加
- oddwatcher - ドキュメント追加と改善
- mihomoQ - コード修正とドキュメント改善
- marcelodotnet - ドキュメント追加と改善

## 🌏 翻訳者

- czoins - アラビア語
- yuhao2348732, 0x391F, nkh0472, yfdyh000, gexgd0419, Zerorigin, UnnamedOrange, DevSplash, Becods, okrc, 4rt3mi5, sepcnt, fzxx, Vstory, GT-Stardust, habatake, mihomoQ - 簡体字中国語
- TragicLifeHu, Hulen, xiongsp, habatake, mihomoQ - 繁体字中国語
- RockyTDR - オランダ語
- imalyzer - ペルシア語
- clexanis, Mmoi-Fr, hippalectryon-0, Monsieur Pissou - フランス語（メールで提供）
- bastik-1001, APMichael - ドイツ語
- timinoun - ハンガリー語（メールで提供）
- isaak654, DerivativeOfLog7 - イタリア語
- takahiro-itou, lllIIIlll - 日本語
- VenusGirl, 꿈을꾸는 파랑새(Sakai) - 韓国語
- divinity76 - ノルウェー語（ブークモール）
- 7zip, AndrzejRafalowski - ポーランド語（[別途提供](https://forum.xanasoft.com/threads/polish-translation.4/page-2)）
- JNylson - ポルトガル語およびブラジル ポルトガル語
- lufog, marat2509 - ロシア語
- LumitoLuma, sebadamus - スペイン語
- 1FF, Thatagata - スウェーデン語（メールまたは pull request で提供）
- xorcan, fmbxnary, offhub - トルコ語
- SuperMaxusa, lufog, Nazar1ky - ウクライナ語
- GunGunGun - ベトナム語

翻訳を提出する前に、[翻訳の協力と帰属に関するガイドライン](./TRANSLATING.md#translation-collaboration-and-attribution) と [ローカライズの注意点とヒント](https://github.com/sandboxie-plus/Sandboxie/discussions/1123) をお読みください。

## 📚 ドキュメント翻訳者

- Vstory, GT-Stardust, wzxjohn, SOLEADO20, habatake, YuHuanTin, darkskygit, oddwatcher - 簡体字中国語

ドキュメント翻訳者は全員、翻訳を送信する前に [多言語翻訳コントリビューション ガイド](https://github.com/sandboxie-plus/sandboxie-docs/issues/175#issuecomment-2840258519) を確認することをお勧めします。
