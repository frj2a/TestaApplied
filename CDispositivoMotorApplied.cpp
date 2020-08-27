// "$Date: 2018-03-23 13:11:01 -0300 (sex, 23 mar 2018) $"
// "$Author: fares $"
// "$Revision: 4195 $"

#include "CDispositivoMotorApplied.h"

#include "../QtCliche/NomesDispositivos.h"
#include <QTextCodec>
#include <QApplication>
#include <cmath>

#if defined(_DEBUG) && defined(USE_QDEBUG)
#  include <QDebug>
#  define DEBUG_QDEBUG(x) qDebug() << x
#else
#  define DEBUG_QDEBUG(x)
#endif

#if defined(_DETALHE_MOTOR_APPLIED)
#   include "Erros.h"
#	define LOG_DETALHE(a,b,c)	emit Log(a,b,c);
#else
#	define LOG_DETALHE(a,b,c)	{ ; }
#endif
#if defined(_DEBUG)
#   include "Erros.h"
#	define LOG_DEBUG(a,b,c)	emit Log(a,b,c);
#else
#	define LOG_DEBUG(a,b,c)	{ ; }
#endif

// também definidas nas classes CDispositivoMotorTeste e CTelaDiagnostico
#define	ENTRADA_PEDAL_DIREITO	0
#define	ENTRADA_PEDAL_ESQUERDO	1
#define	ENTRADA_FIM_CURSO		3	/* começando em 1  */
#define	ENTRADA_PRESSOSTATO	8	/* chamada de "X8" */
#define	SAIDA_TRAVA_ELEVADOR	0
#define	SAIDA_TRAVA_FREIO		3


//	máximo de pulsos para dar voltas inteiras: 2147468400 = 42.273 voltas, aproximadamente igual a 1,85 km

CDispositivoMotorApplied::CDispositivoMotorApplied(CDadosConfiguracao * configuracao, QObject *pai) : IDispositivoMotor(configuracao, pai) {
	QTextCodec *codec = QTextCodec::codecForName("UTF-8"); //	UTF-8	ISO-8859-1   System    latin1 	latin9  	windows-1251

	mNomeDispositivo = NOME_ACESSO_DISPOSITIVO_DESC;
	mTipoDispositivo = IDispositivoMotor::TipoNulo;
	mPosicaoEncoderDesejadaTolerancia = 1;
	mRestringindoPortas = false;
	mConfigurandoTipo = false;

	mTerminador = '\r';
	mCaracteresEspeciais = "\r%?*";
	mMensagemComandoReconhecido = "%";

	mComandoDefeituoso.clear();

	mAlarmesMensagens.clear();
	mAlarmesMensagens << codec->toUnicode("Limite de Posição")
					  << codec->toUnicode("Limite Horario")
					  << codec->toUnicode("Limite Antiorario")
					  << codec->toUnicode("Sobre Temperatura")
					  << codec->toUnicode("Tensão Interna")
					  << codec->toUnicode("Sobre Tensão")
					  << codec->toUnicode("Baixa Tensão")
					  << codec->toUnicode("Sobre corrente")
					  << codec->toUnicode("Motor aberto")
					  << codec->toUnicode("Defeito no Encoder")
					  << codec->toUnicode("Erro de comunicação")
					  << codec->toUnicode("Erro na memória")
					  << codec->toUnicode("Impossível mover")
					  << codec->toUnicode("Não usado 1")
					  << codec->toUnicode("Segmento Q apagado")
					  << codec->toUnicode("Não usado 2") ;
	mAlarmesEstados.clear();
	mAlarmesEstados << false << false << false << false << false << false << false << false << false << false << false << false << false << false << false << false ;

	mEstadosMensagens.clear();
	mEstadosMensagens << codec->toUnicode("Motor habilitado")
					  << codec->toUnicode("Amostrando")
					  << codec->toUnicode("Falha no dispositivo")	// verificar alarmes
					  << codec->toUnicode("Em posicao")
					  << codec->toUnicode("Movendo")
					  << codec->toUnicode("Em jog")
					  << codec->toUnicode("Parando")
					  << codec->toUnicode("Aguardando entrada digital")
					  << codec->toUnicode("Salvando parâmetro")
					  << codec->toUnicode("Alarme")	// verificar alarmes
					  << codec->toUnicode("Localizando home")
					  << codec->toUnicode("Aguardando tempo")
					  << codec->toUnicode("Assistente rodando")
					  << codec->toUnicode("Verificando encoder")
					  << codec->toUnicode("Rodando programa Q")
					  << codec->toUnicode("Inicializando") ;
/*
	Hex 	 Status Code
   Value	bit definition
   =====	==============================================
   0001 	Motor Enabled (Motor Disabled if this bit = 0)
   0002 	Sampling (for Quick Tuner)
   0004 	Drive Fault (check Alarm Code)
   0008 	In Position (motor is in position)
   0010 	Moving (motor is moving)
   0020 	Jogging (currently in jog mode)
   0040 	Stopping (in the process of stopping from a stop command)
   0080 	Waiting (for an input; executing a WI command)
   0100 	Saving (parameter data is being saved)
   0200 	Alarm present (check Alarm Code)
   0400 	Homing (executing an SH command)
   0800 	Waiting (for time; executing a WD or WT command)
   1000 	Wizard running (Timing Wizard is running)
   2000 	Checking encoder (Timing Wizard is running)
   4000 	Q Program is running
   8000 	Initializing (happens at power up) ; Servo Ready (for SV200 drives only)
*/

	mEstadosEstados.clear();
	mEstadosEstados << false << false << false << false << false << false << false << false << false << false << false << false << false << false << false << false ;

	mSaidasMensagens.clear();
	mSaidasMensagens[IDispositivoMotor::TipoCamera1]	<< codec->toUnicode("Y1")
														<< codec->toUnicode("Y2")
														<< codec->toUnicode("Y3")
														<< codec->toUnicode("Y4") ;
	mSaidasMensagens[IDispositivoMotor::TipoCamera2]	<< codec->toUnicode("Y1")
														<< codec->toUnicode("Y2")
														<< codec->toUnicode("Y3")
														<< codec->toUnicode("Y4") ;
	mSaidasMensagens[IDispositivoMotor::TipoCilindro]	<< codec->toUnicode("Y1 / ELEVADOR")
														<< codec->toUnicode("Y2")
														<< codec->toUnicode("Y3")
														<< codec->toUnicode("Y4 / FREIO") ;
	mSaidasMensagens[IDispositivoMotor::TipoIndefinido] << codec->toUnicode("Y1")
														<< codec->toUnicode("Y2")
														<< codec->toUnicode("Y3")
														<< codec->toUnicode("Y4") ;
	mSaidasMensagens[IDispositivoMotor::TipoNulo]       << codec->toUnicode("Y1")
														<< codec->toUnicode("Y2")
														<< codec->toUnicode("Y3")
														<< codec->toUnicode("Y4") ;
	mSaidasEstados.clear();
	mSaidasEstados << true << true << true << true;

	mEntradasMensagens.clear();
	mEntradasMensagens[IDispositivoMotor::TipoCamera1]	  << codec->toUnicode("X1")
														  << codec->toUnicode("X2")
														  << codec->toUnicode("X3 / FIM DE CURSO")
														  << codec->toUnicode("X4")
														  << codec->toUnicode("X5")
														  << codec->toUnicode("X6")
														  << codec->toUnicode("X7")
														  << codec->toUnicode("X8") ;
	mEntradasMensagens[IDispositivoMotor::TipoCamera2]	  << codec->toUnicode("X1")
														  << codec->toUnicode("X2")
														  << codec->toUnicode("X3 / FIM DE CURSO")
														  << codec->toUnicode("X4")
														  << codec->toUnicode("X5")
														  << codec->toUnicode("X6")
														  << codec->toUnicode("X7")
														  << codec->toUnicode("X8") ;
	mEntradasMensagens[IDispositivoMotor::TipoCilindro]   << codec->toUnicode("X1 / PEDAL DIREITO")
														  << codec->toUnicode("X2 / PEDAL ESQUERDO")
														  << codec->toUnicode("X3 / FIM DE CURSO")
														  << codec->toUnicode("X4")
														  << codec->toUnicode("X5")
														  << codec->toUnicode("X6")
														  << codec->toUnicode("X7")
														  << codec->toUnicode("X8 / PRESSOSTATO") ;
	mEntradasMensagens[IDispositivoMotor::TipoIndefinido] << codec->toUnicode("X1")
														  << codec->toUnicode("X2")
														  << codec->toUnicode("X3")
														  << codec->toUnicode("X4")
														  << codec->toUnicode("X5")
														  << codec->toUnicode("X6")
														  << codec->toUnicode("X7")
														  << codec->toUnicode("X8") ;
	mEntradasMensagens[IDispositivoMotor::TipoNulo]       << codec->toUnicode("X1")
														  << codec->toUnicode("X2")
														  << codec->toUnicode("X3")
														  << codec->toUnicode("X4")
														  << codec->toUnicode("X5")
														  << codec->toUnicode("X6")
														  << codec->toUnicode("X7")
														  << codec->toUnicode("X8") ;
	mEntradasEstados.clear();
	mEntradasEstados<< false << false << false << false << false << false << false << false;

	mAceleracao = 2000;	// a menor, entre as câmeras e o cilindro (no caso, cilindro);

	mDefasagemCentroLente = 32000;
	mDistanciaReferenciaRelativa = 500000;
	mDividendoEncoder_uM = 1;
	mDivisorEncoder_uM = 5;
	mDividendoMotorEncoder = 999952;
	mDivisorMotorEncoder = 43980186;	//	dividindo-se 43980186 por 999952 se obtem muito aproximadamente a PI * 14 (diâmetro primitivo)

	mCorrenteMovimento = 200;	// a menor, entre as câmeras e o cilindro (no caso, câmera);
	mCorrenteParado = 180;		// a menor, entre as câmeras e o cilindro (no caso, câmera);

	mMargemMinimaNegativa = 39000;
	mMargemMinimaPositiva = 39000;
	mMovimentoGrande = 1000;
	mMovimentoMedio = 100;
	mMovimentoPequeno = 10;
	mPosicaoReferenciaAbsoluta = 0;	// 283860 para a câmera 2, na travessa de desenvolvimento.

	mResolucaoMotor = 50800;
	mSensorAtivado = 0;

	mVelocidadeAlta = 132000;	//	7040; 	// 3520;	// 1760;
	mVelocidadeBaixa = 5500;	//	2200;	//	440;
	mVelocidadeMedia = mVelocidade = mVelocidadeAntiga = 80000;	//	5280;	//	3520;	// 1760; 	// 880;
	mVelocidadeMaxima = 176000;	//	7744;
	mVelocidadeMinima = 880;

	mVelocidadeIntensidade = IDispositivoMotor::VelocidadeMedia;

	mTimerProssegue.setSingleShot(true);
	mTimerProssegue.setInterval(500);
	mTimerProssegue.stop();
	connect (&mTimerProssegue, SIGNAL(timeout()), this, SLOT(slotRestringirListaPortasContinua()));

	mProgramando = false;
	mPronto = true;
	mEtapaTesteVaiVolta = EsperaTerminar;
	mEtapaTesteVaiVoltaAnterior = EsperaTerminar;
	mTimerVaiVolta = new QTimer();
	mTimerVaiVolta->setInterval(2000);
	mTimerVaiVolta->setSingleShot(true);

	mRestringindoPortasIndice = 0;
	mRestringindoPortasListaPortas.clear();
	mRestringindoPortasListaModelos.clear();
	mRestringindoPortasListaTipos.clear();

	DEBUG_QDEBUG(" Criado um DispositivoMotorApplied.");
}

void CDispositivoMotorApplied::restringirListaPortas() {
	mRestringindoPortas = true;
	mEnderecoDispositivo = "?";
	abrirPorta(mPortasDisponiveis.at(mRestringindoPortasIndice));
	// enviaComandoHex("30 ", false, 5);
	enviaComando("SK\r");					//	(imediato) interrompe qualquer comando em "buffer" em progresso e remove qualquer outros comandos em "buffer" da fila;
	enviaComandoHex("4D 4E 0D", false, 1);	//	(imediato) ("MN\r") antiga forma de obter o código referente ao modelo do acionamento.
	enviaComando("HR\r");					//	(não documentado) pelo que entendo, prepara o acionamento para receber comandos em ASCII;
	enviaComandoHex("4D 4E 0D", false, 1);	//	(imediato) ("MN\r") antiga forma de obter o código referente ao modelo do acionamento.
	enviaComando("SK\r");					//	(imediato) interrompe qualquer comando em "buffer" em progresso e remove qualquer outros comandos em "buffer" da fila;
	enviaComando("MV\r");					//	(imediato) número do modelo, conforme tabela abaixo.
	enviaComando("AR\r");					//	(imediato) zera os alarmes.
	enviaComando("WT0.1\r");				//	(em "buffer") aguarda 0.1s (neste caso).
	enviaComando("AX\r");					//	(em "buffer") zera os alarmes - indicado pelo pessoal da Kalatec como mais adequado para eliminar o histórico entre "boots" sucessivos.
	enviaComando("ME\r");					//	(em "buffer") habilita o motor.
	enviaComando("DA\r");					//	(em "buffer") endereço do dispositivo
	enviaComando("MT1\r");					//	(em "buffer") multiprocessamento habilitado
	enviaComando("PR7\r");					//	(em "buffer") protocolo padrão (bit 0), use caractere de endereço (bit 1) e use "ack"/"nack" (bit 2)
	enviaComando("PM2\r");					//	(em "buffer") inicialização em modo Q / SCL (drive enabled)
	enviaComando("CC2\r");					//	(em "buffer") corrente mínima entre a do cilindro e a das câmeras;
	enviaComando("IFD\r");					//	(imediato) formato decimal (e não hexadecimal)
	enviaComando("IO15\r");					//	(imediato) todas saídas a 0
	enviaComando("IS\r");					//	(imediato) lê todas as entradas
	enviaComando("EP0\r");					//	(em "buffer") zera contador de "encoder"
	enviaComando("SP0\r");					//	(em "buffer") zera contador de passos
	/*
	Response will be in the format AAAABBBC, where AAAA is the firmware version, BBB is the model number
	code, and C is the sub-model number code. Model and sub-model number codes are listed below by drive,
	and Examples are given afterward.
	Drive		Firmware	 Model No. Code	 Sub-Model No. Code
	SV7-S          *               011               -
	SV7-Q          *               012               -
	SV7-Si         *               013               -
	STAC6-S        *               041               -
	STAC6-Q	       *               042               -
	STAC6-Si       *               043               -
	STAC6-220-S	   *               044               -
	STAC6-220-Q	   *               045               -
	STAC6-220-Si   *               046               -
	STAC6-C        *               047               -
	STAC6-220-C    *               048               -
	ST5-S          *               020               -
	ST5-Q          *               022               -
	ST5-Si         *               023               -
	ST5-Plus       *               026               -
	ST10-S         *               021               -
	ST10-Q         *               024               -
	ST10-Si        *               025               -
	ST10-Plus      *               027               -
	STM23S-2AN     *               049               A
	STM23S-2AE     *               049               E
	STM23S-2RN     *               049               C
	STM23S-2RE     *               049               G
	STM23S-3AN     *               049               B
	STM23S-3AE     *               049               F
	STM23S-3RN     *               049               D
	STM23S-3RE     *               049               H
	STM23Q-2AN     *               050               A
	STM23Q-2AE     *               050               E
	STM23Q-2RN     *               050               C
	STM23Q-2RE     *               050               G
	STM23Q-3AN     *               050               B
	STM23Q-3AE     *               050               F
	STM23Q-3RN     *               050               D
	STM23Q-3RE     *               050               H
	*/
	fecharPorta();
	mTimerProssegue.start();
	LOG_DETALHE(ADVERTENCIA, "CDispositivoMotorApplied::restringirListaPortas", QString("%1: enviados comandos para encontrar portas com dispositivos, na porta %2: %3").arg(mNomeDispositivo).arg(mPortaNome).arg(mPortasDisponiveis.at(mRestringindoPortasIndice)));
}

void CDispositivoMotorApplied::slotRestringirListaPortasContinua() {
	IDispositivoMotor::tipo t = mTipoDispositivo;	// esta informação é perdida na instrução seguinte;
	if (mEnderecoDispositivo != "?") {
		mRestringindoPortasListaPortas.append(mPortasDisponiveis.at(mRestringindoPortasIndice));
		mRestringindoPortasListaModelos.append(mModeloProduto);
		mRestringindoPortasListaTipos.append(t);
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::slotRestringirListaPortasContinua", QString("%1: dispositivo encontrado com endereço: %2").arg(mNomeDispositivo).arg(mEnderecoDispositivo));
	}
	mRestringindoPortasIndice++;
	if (mRestringindoPortasIndice == mPortasDisponiveis.size()) {
		// mPortasDisponiveis = mRestringindoPortasListaPortas;		// vou precisar da lista de todas portas realmente disponíveis para eventuais diagnósticos;
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::slotRestringirListaPortasContinua", QString("%1: quantidade de dispositivos encontrados: %2").arg(mNomeDispositivo).arg(mRestringindoPortasListaPortas.size()));
		mRestringindoPortasIndice = 0;
		emit sinalDispositivosEncontrados();
		mRestringindoPortas = false;
	} else {
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::slotRestringirListaPortasContinua", QString("%1: solicita nova tentativa de restrição de portas realmente usadas por dispositivos.").arg(mNomeDispositivo));
		restringirListaPortas();
	}
}

void CDispositivoMotorApplied::reiniciarFabricaDispositivo() {
	// enviaComandoHex("30 ", false, 4);						// apenas por ter aberto a porta
	enviaComandoHex("51 54 0D 48 4D 0D 43 43", false, 4);	// aqui começa o restaurar de configuração de fábrica
	enviaComandoHex("43 ", false, 2);
	enviaComandoHex("43 ", false, 2);
	enviaComandoHex("79 8E", false, 4);
	enviaComandoHex("79 92", false, 4);
	enviaComandoHex("79 39", false, 4);
	enviaComandoHex("53 69 31 32 33 34 43", false, 2);
	enviaComandoHex("43 ", false, 2);
	enviaComandoHex("79 8E", false, 4);
	enviaComandoHex("79 92", false, 4);
	enviaComandoHex("79 39", false, 4);
	enviaComandoHex("43 ", false, 2);
	enviaComandoHex("75 ", false, 161);
	enviaComandoHex("43 ", false, 2);
	enviaComandoHex("43 ", false, 2);
	enviaComandoHex("48 52 0D 44 41 0D", false, 6);
	for (int i=1; i<=12; i++) {
		enviaComando(QString("QD%1\r").arg(i));		//	(imediato) apaga a fila especificada;
	}
}

void CDispositivoMotorApplied::configurarParaTipo(IDispositivoMotor::tipo tipo, QString porta) {
	mTipoDispositivo = tipo;
	mNomeDispositivo = tipoEmNome(tipo);
	mConfigurandoTipo = true;

	configurarTipo(tipo, porta);
	bool portaJaAberta = mPortaSerial->isOpen();
	if ( ( portaJaAberta ) && ( mPortaSerial->portName() != porta) ) {
		fecharPorta();
		LOG_DETALHE(ADVERTENCIA, "CDispositivoMotorApplied::configurarParaTipo", QString("%1 : outra porta estava aberta: %2").arg(mNomeDispositivo).arg(mPortaSerial->portName()));
	}
	if ( ! mPortaSerial->isOpen() ) {
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::configurarParaTipo", QString("%1: abrindo porta: %2").arg(mNomeDispositivo).arg(porta));
		abrirPorta(mPortaNome);
	}


	QString enderecoDispositivo;	// durante o processo será lido o endereço, que pode estar errado em relação ao desejado

	// IDispositivoMotor::inicializar();
	switch (tipo) {
		case IDispositivoMotor::TipoCamera1:
			enderecoDispositivo = "5";
			break;
		case IDispositivoMotor::TipoCamera2:
			enderecoDispositivo = "6";
			break;
		case IDispositivoMotor::TipoCamera3:
			enderecoDispositivo = "7";
			break;
		case IDispositivoMotor::TipoCamera4:
			enderecoDispositivo = "8";
			break;
		case IDispositivoMotor::TipoCilindro:
			enderecoDispositivo = "9";
			break;
		case IDispositivoMotor::TipoIndefinido:
			break;
		default:
			break;
	}
	if ( -1 != mRestringindoPortasListaPortas.indexOf( mPortaSerial->portName() ) ) {
		if ( mRestringindoPortasListaTipos.size() > 0 ) {
			mRestringindoPortasListaTipos.replace(mRestringindoPortasListaPortas.indexOf(mPortaSerial->portName()), tipo);
		}
		if (mRestringindoPortasListaModelos.size() > 0) {
			mModeloProduto = mRestringindoPortasListaModelos.at(mRestringindoPortasListaPortas.indexOf(mPortaSerial->portName()));
		}
	}

	enviaComando("SK\r");
	enviaComandoHex( "4D 4E 0D", false, 1); 	//	"MN\r"
	enviaComando("HR\r");
	enviaComandoHex( "4D 4E 0D", false, 1); 	//	"MN\r"
	enviaComando("SK\r");
	enviaComando("MT1\r");
	enviaComando("PR7\r");					//	(em "buffer") protocolo padrão (bit 0), use caractere de endereço (bit 1) e use "ack"/"nack" (bit 2)
	enviaComando("PM2\r\r");
	enviaComando("MR15\r");
	enviaComando("CM21\r");
	enviaComando("IFD\r\r");
	// enviaComando("DA\r");
	enviaComandoHex( "51 54 0D 48 4D 0D 53 43", false, 2);
	enviaComandoHex( "79 8E", false, 4 );	//	inicia o "upload"

	if (mModeloProduto == "ST10-Plus") {
		enviaComandoHex(                                        // ST10-Plus - padrões de fábrica alterados p/ montadora
			"64 00 64 0A 09 60 61 A8 87 04 B0 FF E2 02 58 02 "  // d.d..`a¨?.°ÿâ.X.
			"16 04 B0 17 70 00 05 1F 40 17 70 00 78 00 14 09 "  // ..°.p...@.p.x...
			"60 00 00 02 58 02 58 00 32 00 C8 00 C8 15 02 00 "  // `...X.X.2.È.È...
			"C6 70 31 03 03 03 03 03 03 4E 20 38 52 00 00 00 "  // Æp1......N 8R...
			"07 00 01 00 00 00 00 00 00 03 E8 55 C8 00 D7 08 "  // ..........èUÈ.×.
			"E7 03 E8 1F FF 02 E2 02 15 04 B0 FF E2 03 E8 04 "  // ç.è.ÿ.â...°ÿâ.è.
			"02 0F 64 00 00 14 50 00 00 03 E8 17 70 00 78 63 "  // ..d...P...è.p.xc
			"75 73 74 6F 6D 20 6D 6F 74 6F 72 20 01 00 00 00 "  // ustom motor ....
			"00 00 78 00 01 12 68 00 00 12 50 00 00 03 E8 12 "  // ..x...h...P...è.
			"C0 00 5A 35 30 31 34 2D 38 34 32 20 20 20 20 20 "  // À.Z5014-842
			"25 56", false, 1 );                                // %V
	}
	if (mModeloProduto == "ST5-Plus") {
		enviaComandoHex(                                        // ST5-Plus - padrões de fábrica alterados p/ montadora
			"64 00 64 0A 09 60 61 A8 87 00 00 00 00 01 F4 01 "	//  d.d..`a¨.....ô.
			"BD 04 B0 17 70 00 00 1F 40 17 70 00 78 00 14 09 "	//	½.°.p...@.p.x...
			"60 00 00 02 58 02 58 00 32 00 C8 00 C8 15 02 00 "	//	`...X.X.2.È.È...
			"C6 70 31 03 03 03 03 03 03 4E 20 38 52 00 0A 00 "	//	Æp1......N 8R...
			"07 00 01 00 00 00 00 00 00 03 E8 7C BE 00 37 0A "	//	..........è|¾.7.
			"C0 03 E8 1F FF 03 A8 02 CC 00 00 00 00 01 F4 04 "	//	À.è.ÿ.¨.Ì.....ô.
			"02 13 50 00 00 15 40 00 00 03 E8 17 70 00 78 63 "	//	..P...@...è.p.xc
			"75 73 74 6F 6D 20 6D 6F 74 6F 72 20 01 00 00 00 "	//	ustom motor ....
			"00 00 78 00 01 12 68 00 00 12 50 00 00 03 E8 12 "	//	..x...h...P...è.
			"C0 00 5A 35 30 31 34 2D 38 34 32 20 20 20 20 20 "	//	À.Z5014-842
			"21 33", false, 1);                               	//	!3
	}
	if ( ( mModeloProduto == "ST10-Q" ) && ( tipo != IDispositivoMotor::TipoCilindro ) ) {
		enviaComandoHex(                                        // ST10-Q - padrões de fábrica alterados p/ montadora (câmera)
			"64 00 64 00 09 60 61 A8 87 00 00 00 00 00 C8 00 "  // d.d..`a¨.....È.
			"64 04 B0 17 70 00 00 1F 40 00 00 00 00 00 14 09 "  // d.°.p...@.......
			"60 00 00 02 58 02 58 00 32 00 C8 00 C8 15 02 00 "  // `...X.X.2.È.È...
			"4E 20 31 03 03 03 03 03 03 4E 20 38 52 00 0A 00 "  // N 1......N 8R...
			"05 00 01 00 00 00 00 00 00 01 90 3E 7E 00 82 12 "  // ..........>~..
			"94 03 E8 4A 50 01 71 01 8B 00 00 00 00 00 64 00 "  // .èJP.q......d.
			"01 10 76 00 00 0E 4B 00 00 03 E8 17 70 00 78 63 "  // ..v...K...è.p.xc
			"75 73 74 6F 6D 20 6D 6F 74 6F 72 20 00 03 20 FF "  // ustom motor .. ÿ
			"9C 00 C8 00 01 11 66 00 00 11 6C 00 00 03 E8 0B "  // .È...f...l...è.
			"40 00 3A 48 54 31 37 2D 30 37 31 2F 32 37 31 20 "  // @.:HT17-071/271
			"1E B5", false, 1 );                                // .µ
	}
	if ( ( mModeloProduto == "ST10-Q" ) && ( tipo == IDispositivoMotor::TipoCilindro ) ) {
		enviaComandoHex(                                        // ST10-Q - padrões de fábrica alterados p/ montadora (cilindro)
			"64 00 64 00 09 60 61 A8 87 00 00 00 00 02 58 02 "  // d.d..`a¨.....X.
			"1C 04 B0 17 70 00 00 1F 40 00 00 00 00 00 14 09 "  // ..°.p...@.......
			"60 00 00 02 58 02 58 00 32 00 C8 00 C8 15 02 00 "  // `...X.X.2.È.È...
			"4E 20 31 03 03 03 03 03 03 4E 20 38 52 00 0A 00 "  // N 1......N 8R...
			"05 00 01 00 00 00 00 00 00 00 00 7D 00 00 2D 12 "  // ...........}..-.
			"E6 03 E8 4B 98 01 73 01 84 00 00 00 00 00 64 00 "  // æ.èK.s......d.
			"01 10 76 00 00 0E 4B 00 00 03 E8 17 70 00 78 63 "  // ..v...K...è.p.xc
			"75 73 74 6F 6D 20 6D 6F 74 6F 72 20 00 00 00 00 "  // ustom motor ....
			"00 02 58 00 01 0E 58 80 00 0D 5D C0 00 03 E8 17 "  // ..X...X..]À..è.
			"70 00 78 48 54 32 33 2D 36 30 33 20 20 20 20 20 "  // p.xHT23-603
			"1C 91", false, 1 );                                // .
	}
	if (mModeloProduto == "ST5-Q") {
		enviaComandoHex(                                        // ST5-Q - padrões de fábrica alterados p/ montadora
			"64 00 64 00 09 60 61 A8 87 00 00 00 00 00 C8 00 "  // d.d..`a¨.....È.
			"64 04 B0 17 70 00 00 1F 40 00 00 00 00 00 14 09 "  // d.°.p...@.......
			"60 00 00 02 58 02 58 00 32 00 C8 00 C8 15 03 00 "  // `...X.X.2.È.È...
			"4E 20 31 03 03 03 03 03 03 4E 20 38 52 00 0A 00 "  // N 1......N 8R...
			"05 00 01 00 00 00 00 00 00 01 90 3E 5C 00 81 0A "  // ..........>\..
			"0E 03 E8 1F FF 01 BA 02 4C 00 00 00 00 00 64 00 "  // ..è.ÿ.º.L.....d.
			"01 10 76 00 00 0E 4B 00 00 03 E8 17 70 00 78 63 "  // ..v...K...è.p.xc
			"75 73 74 6F 6D 20 6D 6F 74 6F 72 20 00 05 5A FF "  // ustom motor ..Zÿ
			"AB 00 C8 00 01 11 7D 99 9A 10 44 00 00 03 E8 12 "  // «.È...}.D...è.
			"C0 00 44 48 54 31 37 2D 30 37 35 2F 32 37 35 20 "  // À.DHT17-075/275
			"20 99", false, 1 );                                //  
	}

	fecharPorta();
	msleep(1000);				// isto força a dar um tempo para o driver

	abrirPorta(mPortaNome);
	if ( tipo == IDispositivoMotor::TipoCilindro ) {
		enviaComando("HRDA\r");		// força o driver a responder.
		enviaComando("HG1200\r");
		enviaComando("HP-30\r");
		enviaComando("CG13000\r");
		enviaComando("CF503\r");
		enviaComando("SR102\r");
		enviaComando("SV1000\r");
		enviaComando("AM2\r");
		enviaComando("AC1\r");
		enviaComando("DE1\r");
		enviaComando("VE1\r");
		enviaComando("JA0.2\r");
		enviaComando("JL0.2\r");
		enviaComando("JS0.1\r");
		enviaComando("CC6\r");
		enviaComando("CI5.4\r");
		enviaComando("CD1\r");
	} else {
		enviaComando("HRDA\r");		// força o driver a responder.
		enviaComando("HG0\r");
		enviaComando("HP0\r");
		enviaComando("CG13000\r");
		enviaComando("CF503\r");
		enviaComando("SR102\r");
		enviaComando("SV1000\r");
		enviaComando("AM2\r");
		enviaComando("AC1\r");
		enviaComando("DE1\r");
		enviaComando("VE1\r");
		enviaComando("IS\r");
		enviaComando("JA0.2\r");
		enviaComando("JL0.2\r");
		enviaComando("JS0.1\r");
		enviaComando("CC6\r");
		enviaComando("CI1.8\r");
		enviaComando("CD1\r");
		enviaComando("IS\r");
	}
	for (int i=1; i<=12; i++) {
		enviaComando(QString("QD%1\r").arg(i));
	}

	enviaComando(QString("DA%1\r").arg(enderecoDispositivo));
	// enviaComando(QString("DA%1\r").arg(enderecoDispositivo));
	enviaComando("SA\r");
	// enviaComando("DA\r");
	// enviaComando("DA\r");
	enviaComando("PR7\r");					//	(em "buffer") protocolo padrão (bit 0), use caractere de endereço (bit 1) e use "ack"/"nack" (bit 2)

//	mEnderecoDispositivo = enderecoDispositivo;		// não deve ser necessário, uma vez que o último comando deve ajustar isto automaticamente;

	if ( ! portaJaAberta) {	// se não estava aberta, fecha para ficar como estava antes;
		fecharPorta();
	}
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::configurarParaTipo", QString("%1: dispositivo alterado para: %2").arg(mNomeDispositivo).arg(tipoEmNome(mTipoDispositivo)));
}

bool CDispositivoMotorApplied::inicializar() {
	mEnderecoDispositivo = "?";

	enviaComandoHex("30 ", false, 5);
	enviaComando("SK\r");					//	(imediato) interrompe qualquer comando em "buffer" em progresso e remove qualquer outros comandos em "buffer" da fila;
	enviaComandoHex("4D 4E 0D", false, 1);	//	(imediato) ("MN\r") antiga forma de obter o código referente ao modelo do acionamento.
	enviaComando("HR\r");					//	(não documentado) pelo que entendo, prepara o acionamento para receber comandos em ASCII;
	enviaComandoHex("4D 4E 0D", false, 1);	//	(imediato) ("MN\r") antiga forma de obter o código referente ao modelo do acionamento.
	enviaComando("SK\r");					//	(imediato) interrompe qualquer comando em "buffer" em progresso e remove qualquer outros comandos em "buffer" da fila;
	enviaComando("MV\r");					//	(imediato) número do modelo;
	// enviaComando("DA\r");					//	(em "buffer") endereço do dispositivo;
	// enviaComando("EP0\r");					//	(em "buffer") ajusta posição inicial do contador de "encoder";
	// enviaComando("SP0\r");					//	(em "buffer") ajusta posição inicial do contador de passos;
	enviaComando("MT1\r");					//	(em "buffer") multiprocessamento habilitado;
	enviaComando("PR7\r");					//	(em "buffer") protocolo padrão (bit 0), use caractere de endereço (bit 1) e use "ack"/"nack" (bit 2);
	enviaComando("PM2\r");					//	(em "buffer") inicialização em modo Q / SCL (drive enabled);
	enviaComando("IFD\r");					//	(imediato) formato decimal (e não hexadecimal);
	enviaComando("IO15\r");					//	(imediato) todas saídas a 1;
	// enviaComando("SA\r");

	mTorqueLigado = true;
	usarVelocidadeMedia();
	zerarAlarme();

	// IDispositivoMotor::inicializar();
	mMutex.unlock();
	mInicializado = true;
	mEtapaTesteVaiVolta = EsperaTerminar;
	return true;
}

void CDispositivoMotorApplied::finalizar() {
#ifndef	QTSERIALTEST	// definido apenas no arquivo do projeto "QtSerialTest"
	IDispositivoMotor * classeBase =  static_cast<IDispositivoMotor*> (this) ;
	classeBase->desconectarSinaisConfiguracao();
#endif
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::finalizar", QString("%1: vai finalizar.").arg(mNomeDispositivo));
	// slotPararMovimentoContinuo(mTipoDispositivo);
	mPosicaoEncoderAntiga = mPosicaoEncoder;
	qint32 posicaoPassosDesejada;
	switch (mTipoDispositivo) {
		case IDispositivoMotor::TipoCamera1:
			posicaoPassosDesejada = qint32( std::round( qreal(mPosicaoReferenciaAbsoluta) * mCoeficientePos_uMParaPosPassos ) ) + mResolucaoMotor;
			movePosicaoPassosAbsoluta(posicaoPassosDesejada);
			break;
		case IDispositivoMotor::TipoCamera2:
			posicaoPassosDesejada = qint32( std::round( qreal(mPosicaoReferenciaAbsoluta) * mCoeficientePos_uMParaPosPassos ) ) - mResolucaoMotor;
			movePosicaoPassosAbsoluta(posicaoPassosDesejada);
			break;
		case IDispositivoMotor::TipoCilindro:
			posicaoPassosDesejada = qint32( std::round( qreal(mPosicaoReferenciaAbsoluta) * mCoeficientePosAnguloParaPosPassos ) ) ;
			break;
		case IDispositivoMotor::TipoIndefinido:
			posicaoPassosDesejada = qint32( std::round( qreal(mPosicaoReferenciaAbsoluta) * mCoeficientePos_uMParaPosPassos ) );
			break;
		default:
			posicaoPassosDesejada = qint32( std::round( qreal(mPosicaoReferenciaAbsoluta) * mCoeficientePos_uMParaPosPassos ) );
	}
	// fecharPorta();	// força aguardar a fila de comandos ser esvaziada;
	// abrirPorta(mPortaNome);
	mInicializado = false;
	mPronto = false;
}

bool CDispositivoMotorApplied::valorEntradaFimCurso(QString *mensagem) {
	return valorEntrada(ENTRADA_FIM_CURSO, mensagem);
}

bool CDispositivoMotorApplied::valorEntradaPressostato(QString *mensagem) {
	if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
		return valorEntrada(ENTRADA_PRESSOSTATO, mensagem);
	} else {
		return false;
	}
}

CDispositivoMotorApplied::~CDispositivoMotorApplied() {
	mTimerVaiVolta->stop();
	disconnect (&mTimerProssegue, SIGNAL(timeout()), this, SLOT(slotRestringirListaPortasContinua()));
	delete mTimerVaiVolta;
	if ( mPortaSerial->isOpen() ) {
		fecharPorta();
	}
	DEBUG_QDEBUG("Destruindo um DispositivoMotorApplied."<< mNomeDispositivo);
}

void CDispositivoMotorApplied::efetivarConfiguracao() {
	qint32 tmpAceleracao;
	switch (mTipoDispositivo) {
		case IDispositivoMotor::TipoCamera1:
		case IDispositivoMotor::TipoCamera2:
			tmpAceleracao = qint32( std::round( double(mAceleracao) * double(mDividendoMotorEncoder) / double(mDivisorMotorEncoder) * 10000.0 / 1000.0));	// o "10.000" é para cortar os dígitos além de 4 casas depois da vírgula e o "1.000" é o da regra de conversão do número inteiro da aceleração em ponto flutuante;
			break;
		case IDispositivoMotor::TipoCilindro:
			tmpAceleracao = qint32( std::round( double(mAceleracao) * double(mDividendoMotorEncoder) / double(mDivisorMotorEncoder) / 60.0 * 10000.0 / 1000.0 ) );	// o "10.000" é para cortar os dígitos além de 4 casas depois da vírgula e o "1.000" é o da regra de conversão do número inteiro da aceleração em ponto flutuante;
			break;
		case IDispositivoMotor::TipoIndefinido:
			tmpAceleracao = 20000;
			break;
		default:
			tmpAceleracao = 20000;
	}
	if (tmpAceleracao < 167) {
		tmpAceleracao = 167;		// limite mínimo para os ST-10 e ST-5
	}
	if (tmpAceleracao > 5461167) {
		tmpAceleracao = 5461167;	// limite máximo para os ST-10 e ST-5
	}
	double acel = double(tmpAceleracao) / 10000;	// aqui está novamente o "10.000" de cima.
	enviaComando(QString("AC%1\r").arg(acel));
	enviaComando(QString("DE%1\r").arg(acel));
	enviaComando(QString("JA%1\r").arg(acel));
	enviaComando(QString("JL%1\r").arg(acel));
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::efetivarConfiguracao", QString("%1: aceleração ajustada para %2.").arg(mNomeDispositivo).arg(acel));

	usarVelocidade(mVelocidade);
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::efetivarConfiguracao", QString("%1: velocidade ajustada: %2").arg(mNomeDispositivo).arg(mVelocidade));

	enviaComando(QString("CC%1\r").arg(ajustaValor(mCorrenteMovimento,2)));
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::efetivarConfiguracao", QString("%1: corrente em movimento ajustada para %2.").arg(mNomeDispositivo).arg(mCorrenteMovimento));

	enviaComando(QString("CD%1\r").arg(ajustaValor(mCorrenteParado,2)));
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::efetivarConfiguracao", QString("%1: corrente em repouso ajustada para %2.").arg(mNomeDispositivo).arg(mCorrenteParado));

	int codigo;
	switch (mResolucaoMotor) {
		case 200:
			codigo = 0;
			break;
		case 400:
			codigo = 1;
			break;
		case 2000:
			codigo = 3;
			break;
		case 5000:
			codigo = 4;
			break;
		case 10000:
			codigo = 5;
			break;
		case 12800:
			codigo = 6;
			break;
		case 18000:
			codigo = 7;
			break;
		case 20000:
			codigo = 8;
			break;
		case 21600:
			codigo = 9;
			break;
		case 25000:
			codigo = 10;
			break;
		case 25400:
			codigo = 11;
			break;
		case 25600:
			codigo = 12;
			break;
		case 36000:
			codigo = 13;
			break;
		case 50000:
			codigo = 14;
			break;
		case 50800:
			codigo = 15;
			break;
		default:
			codigo = 15;
	}
	enviaComando(QString("MR%1\r").arg(codigo));

	if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
		mCoeficientePosEncoderParaPosPassos = ( qreal(mResolucaoMotor) * qreal(mDivisorEncoder_uM) * qreal(mDividendoMotorEncoder) ) / ( qreal(mDividendoEncoder_uM) * qreal(mDivisorMotorEncoder) ) / 360000.0;	// divide por 1000 pois o mDividendoEncoder_uM e o mDivisorEncoder_uM tratam de um (micrômetro) e não de mm.
	} else {
		mCoeficientePosEncoderParaPosPassos = ( qreal(mResolucaoMotor) * qreal(mDivisorEncoder_uM) * qreal(mDividendoMotorEncoder) ) / ( qreal(mDividendoEncoder_uM) * qreal(mDivisorMotorEncoder) ) / 1000.0;	// divide por 1000 pois o mDividendoEncoder_uM e o mDivisorEncoder_uM tratam de um (micrômetro) e não de mm.
	}
	mCoeficientePos_uMParaPosEncoder = qreal(mDividendoEncoder_uM) / qreal(mDivisorEncoder_uM);
	mCoeficientePos_uMParaPosPassos = ( qreal(mResolucaoMotor) * qreal(mDividendoMotorEncoder)) / qreal(mDivisorMotorEncoder) / 1000.0 ;
	mCoeficientePosAnguloParaPosPassos = qreal(mResolucaoMotor) * qreal(mDividendoMotorEncoder) / qreal(mDivisorMotorEncoder) / 360000.0 ;
	mCoeficientePosAnguloParaPosEncoder = 1.0;

	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::efetivarConfiguracao", QString("%1: resolução do motor ajustada para %2 pulsos por volta.").arg(mNomeDispositivo).arg(mResolucaoMotor));

	// mSensorAtivado ;	// não faço ideia do que é isto !!!!!!!!!!!!!!!!!!!!!!!!

	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::efetivarConfiguracao", QString("%1: parâmetros de configuração passados ao dispositivo.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::tratarResposta() { // QByteArray bufferEntrada, QByteArray bufferSaida) {
	if (mPronto && ( ! mFilaComandos.isEmpty())) {
		mComandoReconhecido = true;
		if ( mFilaComandos.first().mEspecialUsuario ) {
			emit sinalRespostaAComandoUsuario(mTipoDispositivo, mBufferRecebido);
		}
		if (mFilaComandos.first().mBinario) {
			mComandoReconhecido = true;
			mBufferSaida.clear();
			mBufferRecebido.clear();
			// estamos dentro de um "if" que consulta o "mFilaComandos.first().mBinario"
			// LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: Comando aceito: ").arg(mNomeDispositivo), mFilaComandos.first().mBinario ? mBufferSaida.toHex() : mBufferSaida.simplified() );
			LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: Comando aceito: %2").arg(mNomeDispositivo).arg(QString(mBufferSaida.toHex())));
		} else {
			if ( mBufferRecebido.contains(mMensagemComandoReconhecido.toLatin1()) ) {
				// estamos dentro do "else" de um "if" que consulta o "mFilaComandos.first().mBinario"
				// LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: Comando aceito: ").arg(mNomeDispositivo), mFilaComandos.first().mBinario ? mBufferSaida.toHex() : mBufferSaida.simplified() );
				LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: Comando aceito: %2").arg(mNomeDispositivo).arg(QString(mBufferSaida.simplified())));
				mComandoReconhecido = true;
			} else {
				// QByteArray lixo = mBufferRecebido;
				if (mBufferRecebido.contains("?")) {
					if (! mBufferSaida.contains("HR")) {
						if (mBufferRecebido.contains("?7")) {
							mComandoDefeituoso = mBufferSaida;
							erroGrave();
							return;
						} else {
							mComandoReconhecido = false;
							// estamos dentro do "else" de um "if" que consulta o "mFilaComandos.first().mBinario"
							// LOG_DETALHE(ADVERTENCIA, "CDispositivoMotorApplied::tratarResposta", QString("%1: COMANDO NÃO COMPREENDIDO: ").arg(mNomeDispositivo), mFilaComandos.first().mBinario ? mBufferSaida.toHex() : mBufferSaida.simplified() );
							LOG_DETALHE(ADVERTENCIA, "CDispositivoMotorApplied::tratarResposta", QString("%1: COMANDO NÃO COMPREENDIDO: %2").arg(mNomeDispositivo).arg(QString(mBufferSaida.simplified())));
						}
					}
				} else {
					if ( mBufferRecebido.size() > 4) {
						if (mBufferSaida.contains("MV")) {
							mModeloProduto = mBufferRecebido.mid(4,3);
							int tipo = mModeloProduto.toInt();
							switch (tipo) {		// apenas alguns...
								case 20:
									mModeloProduto = "ST5-S";
									break;
								case 22:
									mModeloProduto = "ST5-Q";
									break;
								case 23:
									mModeloProduto = "ST5-Si";
									break;
								case 26:
									mModeloProduto = "ST5-Plus";
									break;
								case 21:
									mModeloProduto = "ST10-S";
									break;
								case 24:
									mModeloProduto = "ST10-Q";
									break;
								case 25:
									mModeloProduto = "ST10-Si";
									break;
								case 27:
									mModeloProduto = "ST10-Plus";
									break;
								default:
									mModeloProduto = QString("ignorado (%1)").arg(mModeloProduto);
							}
						} else {
							int desloc = 0;
							if (0x3F > mBufferRecebido.at(0)) {		// os endereços são números (0x30 a 0x39), e as respostas são letras (0x40 em diante)
								desloc = 1;
							}
							switch (mBufferRecebido.at(0 + desloc)) {
								case 'A' :
									switch (mBufferRecebido.at(1 + desloc)) {
										case 'L' :		// "AL"  ***************************
											if (mBufferRecebido.contains('=')) {
												mAlarmes = QString(mBufferRecebido).split("=").at(1).simplified().toUShort();
												{
													bool * alarmes = mAlarmesEstados.data();
													alarmes[ 0] = /* mAlarmeLimitePosicao  		= */ ( ( mAlarmes & 0x0001 ) > 0 );
													alarmes[ 1] = /* mAlarmeLimiteHorario  		= */ ( ( mAlarmes & 0x0002 ) > 0 );
													alarmes[ 2] = /* mAlarmeLimiteContraHorario	= */ ( ( mAlarmes & 0x0004 ) > 0 );
													alarmes[ 3] = /* mAlarmeSobreTemperatura   	= */ ( ( mAlarmes & 0x0008 ) > 0 );
													alarmes[ 4] = /* mAlarmeTensaoInterna  		= */ ( ( mAlarmes & 0x0010 ) > 0 );
													alarmes[ 5] = /* mAlarmeSobreTensao			= */ ( ( mAlarmes & 0x0020 ) > 0 );
													alarmes[ 6] = /* mAlarmeBaixaTensao			= */ ( ( mAlarmes & 0x0040 ) > 0 );
													alarmes[ 7] = /* mAlarmeSobreCorrente  		= */ ( ( mAlarmes & 0x0080 ) > 0 );
													alarmes[ 8] = /* mAlarmeMotorAberto			= */ ( ( mAlarmes & 0x0100 ) > 0 );
													alarmes[ 9] = /* mAlarmeDefeitoEncoder 		= */ ( ( mAlarmes & 0x0200 ) > 0 );
													alarmes[10] = /* mAlarmeErroComunicacao		= */ ( ( mAlarmes & 0x0400 ) > 0 );
													alarmes[11] = /* mAlarmeErroMemoria			= */ ( ( mAlarmes & 0x0800 ) > 0 );
													alarmes[12] = /* mAlarmeImpossivelMover		= */ ( ( mAlarmes & 0x1000 ) > 0 );
													alarmes[13] = /* mAlarmeNaoUsado1  			= */ ( ( mAlarmes & 0x2000 ) > 0 );
													alarmes[14] = /* mAlarmeSegmentoQapagado   	= */ ( ( mAlarmes & 0x4000 ) > 0 );
													alarmes[15] = /* mAlarmeNaoUsado2  			= */ ( ( mAlarmes & 0x8000 ) > 0 );
												}
												if (mAlarmesAnteriores != mAlarmes) {
													mAlarmesAnteriores = mAlarmes;
													emit sinalAlarme(mTipoDispositivo, mAlarmes);
													LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: condição de alarme: %2").arg(mNomeDispositivo).arg(mAlarmes, 16));
												}
											}
											break;
										default:
											mComandoReconhecido = false;
									}
									break;
								case 'D' :
									switch (mBufferRecebido.at(1 + desloc)) {
										case 'A' :		// "DA"  ***************************
											if (mBufferRecebido.contains('=')) {
												mEnderecoDispositivo = QString(mBufferRecebido).split("=").at(1).simplified();
												mMensagemComandoReconhecido = QString(mEnderecoDispositivo).append("%");
												LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: descoberto endereço do dispositivo: %2").arg(mNomeDispositivo).arg(mEnderecoDispositivo));
												switch (mEnderecoDispositivo.at(0).toLatin1()) {
													case '5':
														if (mRestringindoPortas || mConfigurandoTipo) {
															mNomeDispositivo = NOME_ACESSO_DISPOSITIVO_CAM1;
															mTipoDispositivo = IDispositivoMotor::TipoCamera1;
															mConfigurandoTipo = false;
														} else {
															if (mTipoDispositivo != IDispositivoMotor::TipoCamera1) {
																emit sinalMudancaTipoDetectado(mTipoDispositivo, IDispositivoMotor::TipoCamera1, mPortaNome);
															}
														}
														break;
													case '6':
														if (mRestringindoPortas || mConfigurandoTipo) {
															mNomeDispositivo = NOME_ACESSO_DISPOSITIVO_CAM2;
															mTipoDispositivo = IDispositivoMotor::TipoCamera2;
															mConfigurandoTipo = false;
														} else {
															if (mTipoDispositivo != IDispositivoMotor::TipoCamera2) {
																emit sinalMudancaTipoDetectado(mTipoDispositivo, IDispositivoMotor::TipoCamera2, mPortaNome);
															}
														}
														break;
													case '9':
														if (mRestringindoPortas || mConfigurandoTipo) {
															mNomeDispositivo = NOME_ACESSO_DISPOSITIVO_CILI;
															mTipoDispositivo = IDispositivoMotor::TipoCilindro;
															mConfigurandoTipo = false;
														} else {
															if (mTipoDispositivo != IDispositivoMotor::TipoCilindro) {
																emit sinalMudancaTipoDetectado(mTipoDispositivo, IDispositivoMotor::TipoCilindro, mPortaNome);
															}
														}
														break;
													default:
														if (mRestringindoPortas || mConfigurandoTipo) {
															mNomeDispositivo = NOME_ACESSO_DISPOSITIVO_DESC;
															mTipoDispositivo = IDispositivoMotor::TipoIndefinido;
															mConfigurandoTipo = false;
														} else {
															if (mTipoDispositivo != IDispositivoMotor::TipoIndefinido) {
																emit sinalMudancaTipoDetectado(mTipoDispositivo, IDispositivoMotor::TipoIndefinido, mPortaNome);
															}
														}
												}
												emit sinalEstadoComunicacao(mTipoDispositivo, true);	// resolveu o problema com comunicação
												LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("Descoberto o tipo do dispositivo: %1").arg(mNomeDispositivo));
											}
											break;
										default:
											mComandoReconhecido = false;
									}
									break;
								case 'I':
									switch (mBufferRecebido.at(1 + desloc)) {
										case 'E' :		// "IE"  ***************************
											if ( (mBufferRecebido.contains('=')) && (mTipoDispositivo != IDispositivoMotor::TipoCilindro) ) {
												mPosicaoEncoder = QString(mBufferRecebido).split("=").at(1).simplified().toInt();
												if (mPosicaoEncoder != mPosicaoEncoderAnterior) {
													mPosicaoEncoderAnterior = mPosicaoEncoder;
												}
												emit sinalPosicaoEncoderAtual(mTipoDispositivo, mPosicaoEncoder);
												mPosicao_uM = qint32( std::floor( qreal(mPosicaoEncoder) / mCoeficientePos_uMParaPosEncoder) );
												emit sinalPosicao_uMAtual(mTipoDispositivo, mPosicao_uM);
												LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: posição de encoder: %2").arg(mNomeDispositivo).arg(mPosicaoEncoder));
											}
											break;
										case 'O' :		// "IO"  ***************************
											if (mBufferRecebido.contains('=')) {
												bool teste;
												mSaidas = QString(mBufferRecebido).split("=").at(1).simplified().toUShort(&teste, 2);
												{
													bool * estados = mSaidasEstados.data();
													estados[ 0] = /* mSaida_00 = */ ( ( mSaidas & 0x0001 ) > 0 );	// elevador no cilindro
													estados[ 1] = /* mSaida_01 = */ ( ( mSaidas & 0x0002 ) > 0 );
													estados[ 2] = /* mSaida_02 = */ ( ( mSaidas & 0x0004 ) > 0 );
													estados[ 3] = /* mSaida_03 = */ ( ( mSaidas & 0x0008 ) > 0 );	// freio no cilindro
												}
												if (mSaidas != mSaidasAnteriores) {
													mSaidasAnteriores = mSaidas;
													emit sinalSaida(mTipoDispositivo, mSaidas);
													LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: saídas digitais: %2").arg(mNomeDispositivo).arg(mSaidas, 2));
												}
											}
											break;
										case 'P' :		// "IP"  ***************************
											if (mBufferRecebido.contains('=')) {
												mPosicaoPassos = QString(mBufferRecebido).split("=").at(1).simplified().toInt();
												if (mPosicaoPassos != mPosicaoPassosAnterior) {
													mPosicaoPassosAnterior = mPosicaoPassos;
													emit sinalPosicaoPassosAtual(mTipoDispositivo, mPosicaoPassos);
													LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: posição de passos: %2").arg(mNomeDispositivo).arg(mPosicaoPassos));
												}
												if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
													qreal posicao_uM = qreal(mPosicaoPassos) / mCoeficientePosAnguloParaPosPassos;
													if (mPosicaoPassos >=0) {
														posicao_uM += 0.5;
														mPosicaoEncoder = qint32( qreal(mPosicaoPassos) / mCoeficientePos_uMParaPosPassos + 0.5 );
													} else {
														posicao_uM -= 0.5;
														mPosicaoEncoder = qint32( qreal(mPosicaoPassos) / mCoeficientePos_uMParaPosPassos - 0.5 );
													}
													emit sinalPosicao_uMAtual(mTipoDispositivo, qint32(posicao_uM));
													emit sinalPosicaoEncoderAtual(mTipoDispositivo, mPosicaoEncoder);
												}

												if ( mMonitorandoPosicaoPassos ) {
													if ( mPosicaoPassos == mPosicaoPassosDesejada) {
														enviaComando("SC\r");
														if ( mMonitorandoPosicaoEncoder && ( mTipoDispositivo != IDispositivoMotor::TipoCilindro) ) {
															if ( abs(mPosicaoEncoderDesejada - mPosicaoEncoder) > mPosicaoEncoderDesejadaTolerancia ) {
																if (mVelocidade != mVelocidadeMedia) {
																	usarVelocidadeMedia();
																}
																movePosicaoEncoderAbsoluta(mPosicaoEncoderDesejada);
															} else {
																usarVelocidade(mVelocidadeAntiga);
																if ( ! mModoTeste ) {
																	mMonitorandoPosicaoPassos = false;
																	LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: Posição desejada de passos atingida: %2").arg(mNomeDispositivo).arg(mPosicaoPassos));
																}
															}
														}
													}
												}
											}
											break;
										case 'S' :		// "IS"  ***************************
											if (mBufferRecebido.contains('=')) {
												bool ok;
												mEntradas = QString(mBufferRecebido).split("=").at(1).simplified().toUShort(&ok,2);
												LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: condição de entradas: %2").arg(mNomeDispositivo).arg(mEntradas, 0, 16));
												{
													bool * estados = mEntradasEstados.data();
													estados[ 0] = /* mEntrada_X1_STEP  		= */ ( ( mEntradas & 0x0001 ) > 0 );
													estados[ 1] = /* mEntrada_X2_DIR   		= */ ( ( mEntradas & 0x0002 ) > 0 );
													estados[ 2] = /* mEntrada_X3_Enable		= */ ( ( mEntradas & 0x0004 ) > 0 );
													estados[ 3] = /* mEntrada_X4_AlarmReset	= */ ( ( mEntradas & 0x0008 ) > 0 );
													estados[ 4] = /* mEntrada_X5   			= */ ( ( mEntradas & 0x0010 ) > 0 );
													estados[ 5] = /* mEntrada_X6_CCWlimit  	= */ ( ( mEntradas & 0x0020 ) > 0 );
													estados[ 6] = /* mEntrada_X7_CWlimit   	= */ ( ( mEntradas & 0x0040 ) > 0 );
													estados[ 7] = /* mEntrada_X0_EncoderZ  	= */ ( ( mEntradas & 0x0080 ) > 0 );
												}
												if (mEntradas != mEntradasAnteriores) {
													mEntradasAnteriores = mEntradas;
												}
												emit sinalEntrada(mTipoDispositivo, mEntradas);
												LOG_DETALHE(INFORMACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: entradas digitais: %2").arg(mNomeDispositivo).arg(mEntradas, 0, 2));
												DEBUG_QDEBUG(QString("%1: + + + + + + + + + + + + + + + + entradas: ").arg(mNomeDispositivo) << QString::number(mEntradas, 2));

											}
											break;
										default:
											mComandoReconhecido = false;
									}
									break;
								case 'M' :
									switch (mBufferRecebido.at(1 + desloc)) {
										case 'E' :		// "ME"  ***************************
											if (mBufferRecebido.contains('&')) {
												mComandoReconhecido = true;
											}
											break;
										case 'V' :		// "MV"  ***************************
											break;
										default:
											mComandoReconhecido = false;
									}
									break;
								case 'S' :
									switch (mBufferRecebido.at(1 + desloc)) {
										case 'C' :		// "SC"  ***************************
											if (mBufferRecebido.contains('=')) {
												mEstados = QString(mBufferRecebido).split("=").at(1).simplified().toUShort();
												LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: condição de estado: %2").arg(mNomeDispositivo).arg(mEstados, 0, 2));
												DEBUG_QDEBUG(QString("%1: estado: ").arg(mNomeDispositivo) << QString::number(mEstados, 2));
												{
													bool * estados = mEstadosEstados.data();
													estados[ 0] = /* mEstadoMotorHabilitado		= */ ( ( mEstados & 0x0001 ) != 0 );
													estados[ 1] = /* mEstadoAmostrando 			= */ ( ( mEstados & 0x0002 ) != 0 );
													estados[ 2] = /* mEstadoErro   				= */ ( ( mEstados & 0x0004 ) != 0 );
													estados[ 3] = /* mEstadoEmPosicao  			= */ ( ( mEstados & 0x0008 ) != 0 );
													estados[ 4] = /* mEstadoMovendo				= */ ( ( mEstados & 0x0010 ) != 0 );
													estados[ 5] = /* mEstadoEmJog  				= */ ( ( mEstados & 0x0020 ) != 0 );
													estados[ 6] = /* mEstadoParando				= */ ( ( mEstados & 0x0040 ) != 0 );
													estados[ 7] = /* mEstadoAguardandoEntrada  	= */ ( ( mEstados & 0x0080 ) != 0 );
													estados[ 8] = /* mEstadoSalvandoParametro  	= */ ( ( mEstados & 0x0100 ) != 0 );
													estados[ 9] = /* mEstadoAlarme 				= */ ( ( mEstados & 0x0200 ) != 0 );
													estados[10] = /* mEstadoLocalizandoHome		= */ ( ( mEstados & 0x0400 ) != 0 );
													estados[11] = /* mEstadoAguardandoTempo		= */ ( ( mEstados & 0x0800 ) != 0 );
													estados[12] = /* mEstadoAssistenteRodando  	= */ ( ( mEstados & 0x1000 ) != 0 );
													estados[13] = /* mEstadoVerificandoEncoder 	= */ ( ( mEstados & 0x2000 ) != 0 );
													estados[14] = /* mEstadoRodandoProgramaQ   	= */ ( ( mEstados & 0x4000 ) != 0 );
													estados[15] = /* mEstadoInicializando  		= */ ( ( mEstados & 0x8000 ) != 0 );
												}
												if (mEstados != mEstadosAnteriores) {
													mEstadosAnteriores = mEstados;
													LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: estados: %2").arg(mNomeDispositivo).arg(mEstados, 2));
												}
												emit sinalEstado(mTipoDispositivo, mEstados);
												mEmMovimento = ( ( ( mEstados & 0x0010 ) != 0 ) /* mEstadoMovendo */ ) ||  ( ( ( mEstados & 0x0020 ) != 0 ) /* mEstadoEmJog */ );
												if ( mEmMovimento != mEmMovimentoAnterior) {
													mEmMovimentoAnterior = mEmMovimento;
													mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
													if ( ( ! mEmMovimento ) && ( ! mProgramando ) ) {
														enviaComando("IS\r");	// pode ter parado por ter atingido um limite (fim-de-curso);
														enviaComando("IE\r");	// pode ter parado por ter atingido a posição desejada de encoder;
														enviaComando("IS\r");	// pode ter parado por ter atingido um limite (fim-de-curso);
														enviaComando("IP\r");	// pode ter parado por ter atingido a posição desejada de passos;
													}
													LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::tratarResposta", QString("%1: estado de movimento: %2 propagado.").arg(mNomeDispositivo).arg(mEmMovimento?"MOVENDO":"PARADO"));
													// DEBUG_QDEBUG(QString("- - - - %1: estado de movimento: %2 propagado;").arg(mNomeDispositivo).arg(mEmMovimento?"MOVENDO":"PARADO"));
													emit sinalDispositivoEmMovimento(mTipoDispositivo, mEmMovimento);
												}
											}
											break;
										default:
											mComandoReconhecido = false;
									}
									break;
								default:	// aqui vem parar as respostas dos comandos que não são usados pelo sistema, como por exemplo os comandos dados pelo usuário;
									mComandoReconhecido = false;
							}
						}
					}
				}
			}
		}
	}
}

void CDispositivoMotorApplied::slotReabrirPorta() {
	abrirPorta(mPortaNome);

	// IDispositivoMotor::inicializar();
	mMutex.unlock();
	mInicializado = true;

	if ( ! mComandoDefeituoso.isEmpty()) {
		enviaComando("QT\r");					//	(não documentado);
		enviaComando("QT\r");					//	(não documentado);
		enviaComando("HR\r");					//	(não documentado)
		enviaComandoHex("4D 4E 0D", false, 1);	//	(imediato) ("MN\r") antiga forma de obter o código referente ao modelo do acionamento.
		enviaComando("MT1\r");					//	(em "buffer") multiprocessamento habilitado
		enviaComando("PR7\r");					//	(em "buffer") protocolo padrão (bit 0), use caractere de endereço (bit 1) e use "ack"/"nack" (bit 2)
		enviaComando("PM2\r");					//	(em "buffer") inicialização em modo Q / SCL (drive enabled)
		enviaComando("IFD\r");					//	(imediato) formato decimal (e não hexadecimal)
		// inicializar();
		enviaComando(mComandoDefeituoso, false, false);
		LOG_DETALHE(ERRO, "CDispositivoMotorApplied::slotReabrirPorta", QString("%1: tornando a abrir a porta com o comando: %2").arg(mNomeDispositivo).arg(QString(mComandoDefeituoso)));
		mComandoDefeituoso.clear();
	} else {
		mRodando = false;
		enviaSolicitacaoEstado();
		enviaComando("MV\r");					//	(imediato) número do modelo, conforme tabela acima.
		// enviaComando("DA\r");					//	(em "buffer") endereço do dispositivo
		LOG_DETALHE(ERRO, "CDispositivoMotorApplied::slotReabrirPorta", QString("%1: tornando a abrir a porta com um dos comandos de leitura de estado. ").arg(mNomeDispositivo));
	}
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
}

void CDispositivoMotorApplied::slotMudancaTipoDetectado(IDispositivoMotor::tipo tipoAntigo, IDispositivoMotor::tipo tipoNovo, QString porta) {
	if ( (porta == mPortaNome) && (tipoAntigo == mTipoDispositivo) ) {
#ifndef	QTSERIALTEST	// definido apenas no arquivo do projeto "QtSerialTest"
		desconectarSinaisConfiguracao();
#endif
		configurarParaTipo(tipoNovo, porta);
	}
}

void CDispositivoMotorApplied::enviaSolicitacaoEstado() {
	if (mEmMovimento) {
		switch (mSeletorEstado) {
			case 0:
				enviaComando("SC\r");	// estados
				mSeletorEstado = 1;
				break;
			case 1:
				if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
					enviaComando("IP\r");	// posicao passos
				} else {
					enviaComando("IE\r");	// posição "encoder"
				}
				mSeletorEstado = 2;
				break;
			case 2:
				enviaComando("IS\r");	// entradas
				mSeletorEstado = 0;
				break;
			default:
				mSeletorEstado = 0;
		}
	} else {
		switch (mSeletorEstado) {
			case 0:
				enviaComando("SC\r");	// estados
				mSeletorEstado = 1;
				break;
			case 1:
				enviaComando("IE\r");	// posição "encoder"
				mSeletorEstado = 2;
				break;
			case 2:
				enviaComando("AL\r");	// alarmes
				mSeletorEstado = 3;
				break;
			case 3:
				enviaComando("IP\r");	// posicao passos
				mSeletorEstado = 4;
				break;
			case 4:
				enviaComando("IS\r");	// entradas
				mSeletorEstado = 5;
				break;
			case 5:
				enviaComando("SC\r");	// estados
				mSeletorEstado = 6;
				break;
			case 6:
				enviaComando("IE\r");	// posição "encoder"
				mSeletorEstado = 7;
				break;
			case 7:
				enviaComando("IO\r");	// saídas
				mSeletorEstado = 8;
				break;
			case 8:
				enviaComando("IP\r");	// posicao passos
				mSeletorEstado = 9;
				break;
			case 9:
				enviaComando("IS\r");	// entradas
				mSeletorEstado = 0;
				break;
			default:
				mSeletorEstado = 0;
				break;
		}
	}
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::enviaSolicitacaoEstado", QString("%1: solicitado o estado do driver.").arg(mNomeDispositivo));
	DEBUG_QDEBUG(QString("%1: solicitado o estado do driver.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::zerarAlarme() {
	enviaComando("AR\r");
	enviaComando("ME\r");
	enviaComando("AL\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::zerarAlarme", QString("%1: alarmes zerados e motor habilitado.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::zerarPassos() {
	enviaComando("EP0\r");
	enviaComando("SP0\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::zerarPassos", QString("%1: posição de contador de passos zerada.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::ajustarPosicao(qint32 posicao_uM) {
	if (posicao_uM >= 0) {
		mPosicaoEncoder = qint32(posicao_uM * mCoeficientePos_uMParaPosEncoder + 0.5 );
		mPosicaoPassos  = qint32(posicao_uM * mCoeficientePos_uMParaPosPassos  + 0.5 );
	} else {
		mPosicaoEncoder = qint32(posicao_uM * mCoeficientePos_uMParaPosEncoder - 0.5 );
		mPosicaoPassos  = qint32(posicao_uM * mCoeficientePos_uMParaPosPassos  - 0.5 );
	}
	enviaComando(QString("EP%1\r").arg(mPosicaoEncoder));
	enviaComando(QString("SP%1\r").arg(mPosicaoPassos));
	enviaComando("SC\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::ajustarPosicao", QString("%1: posição de contador de encoder ajustada para %2 um").arg(mNomeDispositivo).arg(posicao_uM));
}

void CDispositivoMotorApplied::ajustarPosicaoInicialReferencia() {
	switch (mTipoDispositivo) {
		case IDispositivoMotor::TipoCamera1:
			if ( ( mPosicaoReferenciaAbsoluta + mDefasagemCentroLente ) >= 0 ) {
				mPosicaoEncoder = qint32 ( ( mPosicaoReferenciaAbsoluta + mDefasagemCentroLente ) * mCoeficientePos_uMParaPosEncoder + 0.5 );
				mPosicaoPassos  = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePos_uMParaPosPassos + 0.5 ) ;
			} else {
				mPosicaoEncoder = qint32 ( ( mPosicaoReferenciaAbsoluta + mDefasagemCentroLente ) * mCoeficientePos_uMParaPosEncoder - 0.5 );
				mPosicaoPassos  = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePos_uMParaPosPassos - 0.5 ) ;
			}
			break;
		case IDispositivoMotor::TipoCamera2:
			if ( ( mPosicaoReferenciaAbsoluta + mDefasagemCentroLente ) >= 0 ) {
				mPosicaoEncoder = qint32 ( ( mPosicaoReferenciaAbsoluta - mDefasagemCentroLente ) * mCoeficientePos_uMParaPosEncoder + 0.5 );
				mPosicaoPassos  = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePos_uMParaPosPassos + 0.5 );
			} else {
				mPosicaoEncoder = qint32 ( ( mPosicaoReferenciaAbsoluta - mDefasagemCentroLente ) * mCoeficientePos_uMParaPosEncoder + 0.5 );
				mPosicaoPassos  = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePos_uMParaPosPassos + 0.5 );
			}
			break;
		case IDispositivoMotor::TipoCilindro:
			mPosicaoEncoder = mPosicaoReferenciaAbsoluta;
			if (mPosicaoReferenciaAbsoluta >= 0) {
				mPosicaoPassos = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePosAnguloParaPosPassos + 0.5 );
			} else {
				mPosicaoPassos = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePosAnguloParaPosPassos - 0.5 );
			}
			break;
		case IDispositivoMotor::TipoIndefinido:
			if (mPosicaoReferenciaAbsoluta >= 0) {
				mPosicaoEncoder = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePos_uMParaPosEncoder + 0.5 );
			} else {
				mPosicaoEncoder = qint32 ( mPosicaoReferenciaAbsoluta * mCoeficientePos_uMParaPosEncoder - 0.5 );
			}
			break;
		default:
			break;
	}
	enviaComando(QString("EP%1\r").arg(mPosicaoEncoder));
	enviaComando(QString("SP%1\r").arg(mPosicaoPassos));
	enviaComando(QString("EP%1\r").arg(mPosicaoEncoder));
	enviaComando(QString("SP%1\r").arg(mPosicaoPassos));
	enviaComando("IE\r");
	enviaComando("IP\r");
	enviaComando("SC\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::ajustarPosicaoInicialReferencia", QString("%1: posição de contador de encoder retornada à referência absoluta.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::moveContinuoNegativo(qint32 limite_uM) {
	if (limite_uM == 2147483647) {	// este é o maior número para um qint32 e serve como código para se decidir se realmente usa o limite ou se é contínuo mesmo;
		enviaComando("JE\r");
		enviaComando("DI-1\r");
		enviaComando("CJ\r");
		enviaComando("SC\r");
	} else {
		enviaComando(QString("DI%1\r").arg(qint32(limite_uM * mCoeficientePos_uMParaPosPassos)));	// não acho necessário arredondar;
		enviaComando("FP\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	}
	mMonitorandoPosicaoEncoder = false;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::moveContinuoNegativo", QString("%1: iniciado movimento contínuo negativo.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::moveContinuoPositivo(qint32 limite_uM) {
	if (limite_uM == 2147483647) {	// este é o maior número para um qint32 e serve como código para se decidir se realmente usa o limite ou se é contínuo mesmo;
		enviaComando("JE\r");
		enviaComando("DI1\r");
		enviaComando("CJ\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	} else {
		enviaComando(QString("DI%1\r").arg(qint32(limite_uM * mCoeficientePos_uMParaPosPassos)));	// não acho necessário arredondar;
		enviaComando("FP\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	}
	mMonitorandoPosicaoEncoder = false;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::moveContinuoPositivo", QString("%1: iniciado movimento contínuo positivo.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::slotPararMovimentoContinuo(IDispositivoMotor::tipo tipoDispositivo) {
	if (tipoDispositivo == mTipoDispositivo) {
		enviaComando("SK\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::slotPararMovimentoContinuo", QString("%1: movimento interrompido.").arg(mNomeDispositivo));
	}
}

void CDispositivoMotorApplied::moveContinuoInterrompe() {
	enviaComando("SJ\r");
	enviaComando("SC\r");
	enviaComando("IS\r");
	enviaComando("SC\r");
	enviaComando("IS\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::moveContinuoInterrompe", QString("%1: movimento interrompido.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::movePosicaoPassosAbsoluta(qint32 posPassosDestino) {
	if (mPosicaoPassos != posPassosDestino) {
		mPosicaoEncoderAntiga = mPosicaoEncoder;
		mPosicaoPassosDesejada = posPassosDestino;
		// usarVelocidadeAlta();
		enviaComando(QString("DI%1\r").arg(posPassosDestino));
		enviaComando("FP\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		mMonitorandoPosicaoPassos = true;
		mMonitorandoPosicaoEncoder = false;
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::movePosicaoPassosAbsoluta", QString("%1: move para a posição de %2 passos.").arg(mNomeDispositivo).arg(posPassosDestino));
	}
}

void CDispositivoMotorApplied::movePosicaoPassosRelativa(qint32 posPassosDestino) {
	if (posPassosDestino != 0) {
		mPosicaoEncoderAntiga = mPosicaoEncoder;
		mPosicaoPassosDesejada = mPosicaoPassos + posPassosDestino;
		// usarVelocidadeAlta();
		enviaComando(QString("DI%1\r").arg(mPosicaoPassosDesejada));
		enviaComando("FP\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		mMonitorandoPosicaoPassos = true;
		mMonitorandoPosicaoEncoder = false;
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::movePosicaoPassosRelativa", QString("%1: move para a posição de %2 passos.").arg(mNomeDispositivo).arg(posPassosDestino));
	}
}

void CDispositivoMotorApplied::movePosicaoEncoderAbsoluta(qint32 posEncoderDestino) {
	if ( abs(posEncoderDestino - mPosicaoEncoder) > mPosicaoEncoderDesejadaTolerancia ) {	// verifica se precisa mesmo andar;
		mPosicaoEncoderAntiga = mPosicaoEncoder;
		mPosicaoEncoderDesejada = posEncoderDestino;
		if ( (mPosicaoEncoderDesejada - mPosicaoEncoder) >= 0 ) {
			mPosicaoPassosDesejada = qint32( ( qreal( mPosicaoEncoderDesejada - mPosicaoEncoder ) * mCoeficientePosEncoderParaPosPassos ) + 0.5 ) + mPosicaoPassos ;
		} else {
			mPosicaoPassosDesejada = qint32( ( qreal( mPosicaoEncoderDesejada - mPosicaoEncoder ) * mCoeficientePosEncoderParaPosPassos ) - 0.5 ) + mPosicaoPassos ;
		}
		// usarVelocidadeAlta();
		// usarVelocidadeMedia();
		enviaComando(QString("DI%1\r").arg(mPosicaoPassosDesejada));
		enviaComando("FP\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		if (mTipoDispositivo != IDispositivoMotor::TipoCilindro) {
			mMonitorandoPosicaoEncoder = true;
		}
		mMonitorandoPosicaoPassos = true;
		// LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::movePosicaoEncoderAbsoluta", QString("%1: move para a posição de encoder de %2 pulsos.").arg(mNomeDispositivo).arg(posEncoderDestino));
	}
}

void CDispositivoMotorApplied::movePosicaoEncoderRelativa(qint32 posEncoder) {
	movePosicaoEncoderAbsoluta(mPosicaoEncoder + posEncoder);
}

void CDispositivoMotorApplied::movePosicaoUmAbsoluta(qint32 posUm) {
	if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
		if (posUm >= 0 ) {
			movePosicaoEncoderAbsoluta( qint32( ( posUm * mCoeficientePosAnguloParaPosEncoder ) + 0.5 ) );
		} else {
			movePosicaoEncoderAbsoluta( qint32( ( posUm * mCoeficientePosAnguloParaPosEncoder ) - 0.5 ) );
		}
	} else {
		if (posUm > 0 ) {
			movePosicaoEncoderAbsoluta( qint32( ( posUm * mCoeficientePos_uMParaPosEncoder) + 0.5 ) );
		} else {
			movePosicaoEncoderAbsoluta( qint32( ( posUm * mCoeficientePos_uMParaPosEncoder) - 0.5 ) );
		}
	}
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::movePosicaoUmAbsoluta", QString("%1: move para a posição %2 um.").arg(mNomeDispositivo).arg(posUm));
}

void CDispositivoMotorApplied::movePosicaoUmRelativa(qint32 posUm) {
	if (posUm >= 0) {
		movePosicaoEncoderRelativa( qint32( (posUm * mCoeficientePos_uMParaPosEncoder ) + 0.5 ) );
	} else {
		movePosicaoEncoderRelativa( qint32( (posUm * mCoeficientePos_uMParaPosEncoder ) - 0.5 ) );
	}
}

void CDispositivoMotorApplied::slotReceberPosicaoDestinoAbsoluta(IDispositivoMotor::tipo tipoDispositivo, qint32 posicaoUmDestinoUm) {
	if (mTipoDispositivo == tipoDispositivo) {
		movePosicaoUmAbsoluta(posicaoUmDestinoUm);
	}
}

void CDispositivoMotorApplied::slotIniciarMovimentoContinuo(IDispositivoMotor::tipo tipoDispositivo, IDispositivoMotor::sentido s, IDispositivoMotor::intensidade v, qint32 limiteSuperior, qint32 limiteInferior) {
	if (mTipoDispositivo == tipoDispositivo) {
		switch ( v ) {
			case IDispositivoMotor::VelocidadeAlta:
				usarVelocidadeAlta();
				break;
			case IDispositivoMotor::VelocidadeMedia:
				usarVelocidadeMedia();
				break;
			case IDispositivoMotor::VelocidadeBaixa:
				usarVelocidadeBaixa();
				break;
		}
		switch ( s ) {
			case IDispositivoMotor::Direita:
				moveContinuoPositivo(limiteInferior);
				break;
			case IDispositivoMotor::Esquerda:
				moveContinuoNegativo(limiteSuperior);
				break;
			default:
				moveContinuoPositivo(limiteInferior);
		}
	}
}

void CDispositivoMotorApplied::ativaSaida(int ordem, bool estado) {
	unsigned char saida = uchar(1 << ordem);
	if (estado) {
		saida = uchar ( mSaidas ) | saida;
	} else {
		saida = uchar ( mSaidas ) & ( ( ~saida) & 0x0f );
	}
	mSaidas = saida;
	enviaComando(QString("IO%1\r").arg(saida));
	QString msg = estado ? "ativada (1)" : "desativada (0)";
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::ativaSaida", QString("%1: saída %2 %4.").arg(mNomeDispositivo).arg(ordem).arg(msg));
	enviaComando("IS\r");
	enviaComando("SC\r");
	enviaComando("IS\r");
	enviaComando("SC\r");
	enviaComando("IS\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
}

void CDispositivoMotorApplied::buscarFimCurso(IDispositivoMotor::sentido direcao, IDispositivoMotor::borda borda, int distanciaDesaceleracao) {
	int entrada = ENTRADA_FIM_CURSO; // a numeração do drive Applied começa em 1;
	distanciaDesaceleracao = abs(distanciaDesaceleracao);
	if (mVelocidade > mVelocidadeBaixa) {
		enviaComando(QString("DI%1\r").arg( (direcao == IDispositivoMotor::Direita) ? - distanciaDesaceleracao : distanciaDesaceleracao));
	} else {
		enviaComando(QString("DI%1\r").arg( (direcao == IDispositivoMotor::Direita) ? "-1" : "1"));	// um único pulso
	}
	enviaComando(QString("FS%1%2\r").arg(entrada).arg( (borda == IDispositivoMotor::Subida) ? 'R':'F'));
	enviaComando("SC\r");
	enviaComando("IS\r");
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	QString msg1 =  (direcao == IDispositivoMotor::Direita) ? " direito" : "esquerdo";
	QString msg2 =  (borda == IDispositivoMotor::Subida) ? " subida" : "descida" ;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::buscarFimCurso", QString("%1: buscar fim de curso %2, com borda de %3.").arg(mNomeDispositivo).arg(msg1).arg(msg2));
}

void CDispositivoMotorApplied::modoTeste(bool modo, QString porta) {	// a porta precisa estar aberta!!!
	mModoTeste = modo;
	QMutex * trava = new QMutex;
	trava->lock();
	if (porta == "") {
		porta = mPortaSerial->portName();
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::modoTeste", QString("usando porta em uso atualmente: %1").arg(porta));
	}
	if ( ( mPortaSerial->isOpen() ) && ( mPortaSerial->portName() != porta) ) {
		fecharPorta();
		LOG_DETALHE(ADVERTENCIA, "CDispositivoMotorApplied::modoTeste", QString("outra porta estava aberta: %1").arg(mPortaSerial->portName()));
	}
	mPortaNome = porta;
	if ( ! mPortaSerial->isOpen() ) {
		abrirPorta(mPortaNome);
		LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::modoTeste", QString("abrindo porta: %1").arg(porta));
	}
	if (mModoTeste) {
		usarVelocidadeMedia();
		slotAceleracao_Mudou(8800);
		mModoTesteSentido = IDispositivoMotor::Direita;
		enviaComando("DI12700\r");
		enviaComando("FL\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mModoTesteTimer.start();
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	} else {
		mModoTesteTimer.stop();
		if (mModoTesteSentido == IDispositivoMotor::Direita) {
			enviaComando("DI-12700\r");
			enviaComando("FL\r");
			enviaComando("SC\r");
			enviaComando("IS\r");
			enviaComando("SC\r");
			enviaComando("IS\r");
			mModoTesteSentido = IDispositivoMotor::Esquerda;
		} else {
			enviaComando("DI12700\r");
			enviaComando("FL\r");
			enviaComando("SC\r");
			enviaComando("IS\r");
			enviaComando("SC\r");
			enviaComando("IS\r");
			mModoTesteSentido = IDispositivoMotor::Direita;
		}
		enviaComando("EP0\r");
		enviaComando("SP0\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		fecharPorta();
	}
	trava->unlock();
	delete trava;
}

void CDispositivoMotorApplied::slotModoTesteTimerTimeout() {
	QMutex * trava = new QMutex;
	trava->lock();
	if (mModoTesteSentido == IDispositivoMotor::Direita) {
		enviaComando("DI-25400\r");
		enviaComando("FL\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		mModoTesteSentido = IDispositivoMotor::Esquerda;
	} else {
		enviaComando("DI25400\r");
		enviaComando("FL\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		enviaComando("SC\r");
		enviaComando("IS\r");
		mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
		mModoTesteSentido = IDispositivoMotor::Direita;
	}
	trava->unlock();
	delete trava;
}

void CDispositivoMotorApplied::slotVelocidadeIntensidade(IDispositivoMotor::tipo tipoDispositivo, IDispositivoMotor::intensidade vel){
	if (tipoDispositivo == mTipoDispositivo) {
		switch (vel) {
			case IDispositivoMotor::VelocidadeAlta:
				usarVelocidadeAlta();
				break;
			case IDispositivoMotor::VelocidadeMedia:
				usarVelocidadeMedia();
				break;
			case IDispositivoMotor::VelocidadeBaixa:
				usarVelocidadeBaixa();
				break;
		}
	}
}

void CDispositivoMotorApplied::liberarDispositivo(IDispositivoMotor::tipo t) {
	if (mTipoDispositivo == t) {
		enviaComando("CC0\r");
		fecharPorta();
	}
}

void CDispositivoMotorApplied::usarVelocidadeAlta() {
	usarVelocidade(mVelocidadeAlta);
	mVelocidadeIntensidade = IDispositivoMotor::VelocidadeAlta;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::usarVelocidadeAlta", QString("%1: velocidade alta selecionada.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::usarVelocidadeBaixa() {
	usarVelocidade(mVelocidadeBaixa);
	mVelocidadeIntensidade = IDispositivoMotor::VelocidadeBaixa;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::usarVelocidadeBaixa", QString("%1: velocidade baixa selecionada.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::usarVelocidadeMedia() {
	usarVelocidade(mVelocidadeMedia);
	mVelocidadeIntensidade = IDispositivoMotor::VelocidadeMedia;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::usarVelocidadeMedia", QString("%1: velocidade média selecionada.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::usarVelocidadeMaxima() {
	usarVelocidade(mVelocidadeMaxima);
	mVelocidadeIntensidade = IDispositivoMotor::VelocidadeAlta;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::usarVelocidadeMaxima", QString("%1: velocidade máxima selecionada.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::usarVelocidadeMinima() {
	usarVelocidade(mVelocidadeMinima);
	mVelocidadeIntensidade = IDispositivoMotor::VelocidadeBaixa;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::usarVelocidadeMinima", QString("%1: velocidade mínima selecionada.").arg(mNomeDispositivo));
}

void CDispositivoMotorApplied::usarVelocidade(qint32 valor) {
	mVelocidadeAntiga = mVelocidade;
	mVelocidade = valor;
	qint32 tmpVelocidade;
	switch (mTipoDispositivo) {
		case IDispositivoMotor::TipoCamera1:
		case IDispositivoMotor::TipoCamera2:
			tmpVelocidade = qint32( double(mVelocidade) * double(mDividendoMotorEncoder) / double(mDivisorMotorEncoder) * 10000.0 / 1000.0 );	// o "10.000" é para cortar os dígitos além de 4 casas depois da vírgula e o "1.000" é o da regra de conversão do número inteiro da velocidade em ponto flutuante;
			break;
		case IDispositivoMotor::TipoCilindro:
			tmpVelocidade = qint32( double(mVelocidade) * double(mDividendoMotorEncoder) / double(mDivisorMotorEncoder) / 60.0 * 10000.0 / 1000.0 );	// o "10.000" é para cortar os dígitos além de 4 casas depois da vírgula e o "1.000" é o da regra de conversão do número inteiro da velocidade em ponto flutuante;
#if defined(_DETALHE_MOTOR_APPLIED) && defined(_DEBUG)
			tmpVelocidade = tmpVelocidade / 6;	// na falta do redutor e/ou das engrenagens: sem nenhum: 60; apenas sem as engrenagens: 6;
#endif
			break;
		case IDispositivoMotor::TipoIndefinido:
			tmpVelocidade = 20000;
			break;
		default:
			tmpVelocidade = 20000;
	}
	if (tmpVelocidade < 42) {
		tmpVelocidade = 42;		// limite mínimo para os ST-10 e ST-5
	}
	if (tmpVelocidade > 800000) {
		tmpVelocidade = 800000;	// limite máximo para os ST-10 e ST-5
	}
	enviaComando(QString("JS%1\r").arg(ajustaValor(tmpVelocidade, 4)));
	enviaComando(QString("VE%1\r").arg(ajustaValor(tmpVelocidade, 4)));
	mSeletorEstado = 0;		// reinicia a tabela de comandos de leitura de estados em geral;
	LOG_DETALHE(DEPURACAO, "CDispositivoMotorApplied::usarVelocidade", QString("%1: velocidade de %2 ajustada.").arg(mNomeDispositivo).arg(double(tmpVelocidade) / 10000));
}

void CDispositivoMotorApplied::travarElevador(bool estado) {
	if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
		ativaSaida(SAIDA_TRAVA_ELEVADOR, ! estado);	// se ativa o freio desativando a saída, no caso do "driver" Applied;
	}
}

void CDispositivoMotorApplied::travarFreio(bool estado) {
	if (mTipoDispositivo == IDispositivoMotor::TipoCilindro) {
		ativaSaida(SAIDA_TRAVA_FREIO, ! estado);		// se ativa o freio desativando a saída, no caso do "driver" Applied;
	}
}

void CDispositivoMotorApplied::iniciarTesteVaiVolta(qint32 quant_uM) {
	connect(this, SIGNAL(sinalPosicaoEncoderAtual(IDispositivoMotor::tipo,qint32)), this, SLOT(slotRespostaComandoTesteVaiVolta(IDispositivoMotor::tipo, qint32)));
	mEtapaTesteVaiVolta = Vai;
	zerarPassos();
	mVaiVoltaQtd = quant_uM;
	mTimerVaiVolta->stop();
	connect(mTimerVaiVolta, SIGNAL(timeout()), this, SLOT(slotObservaPonto()));
	slotRespostaComandoTesteVaiVolta(mTipoDispositivo, 0);
}

void CDispositivoMotorApplied::finalizarTesteVaiVolta() {
	mEtapaTesteVaiVolta = EsperaTerminar;
}

void CDispositivoMotorApplied::slotRespostaComandoTesteVaiVolta(IDispositivoMotor::tipo /* tipoDispositivo */ , qint32 /* pos */ ) {
	if ( ( ! mEmMovimento ) && ( abs(mPosicaoEncoderDesejada - mPosicaoEncoder) <= ( 4 * mPosicaoEncoderDesejadaTolerancia ) ) /* && ( mEtapaTesteVaiVolta != Observa ) */ ) {
		switch (mEtapaTesteVaiVolta) {
			case EsperaTerminar:
				mTimerVaiVolta->stop();
				enviaComando("SK\r", false, false);					//	(imediato) interrompe qualquer movimento, inclusive programa Q;
				disconnect(this, SIGNAL(sinalPosicaoEncoderAtual(IDispositivoMotor::tipo,qint32)), this, SLOT(slotRespostaComandoTesteVaiVolta(IDispositivoMotor::tipo, qint32)));
				disconnect(mTimerVaiVolta, SIGNAL(timeout()), this, SLOT(slotObservaPonto()));
				break;
			case Vai:
				movePosicaoUmAbsoluta(mVaiVoltaQtd);
				mEtapaTesteVaiVoltaAnterior = mEtapaTesteVaiVolta;
				mEtapaTesteVaiVolta = Observa;
				break;
			case Volta:
				movePosicaoUmAbsoluta(0);
				mEtapaTesteVaiVoltaAnterior = mEtapaTesteVaiVolta;
				mEtapaTesteVaiVolta = Observa;
				break;
			case Observa:
				if ( ! mTimerVaiVolta->isActive()) {
					QApplication::beep();
					mTimerVaiVolta->start();
				}
				break;
		}
	}
}

void CDispositivoMotorApplied::slotObservaPonto() {
	switch (mEtapaTesteVaiVoltaAnterior) {
		case Vai:
			mEtapaTesteVaiVolta = Volta;
			break;
		case Volta:
			mEtapaTesteVaiVolta = Vai;
			break;
		default:
			mEtapaTesteVaiVolta = EsperaTerminar;
	}
	slotRespostaComandoTesteVaiVolta(mTipoDispositivo , 0 );
}

#ifdef USE_QDEBUG
#undef USE_QDEBUG
#endif

#ifdef LOG_DETALHE
#undef LOG_DETALHE
#endif

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif

#undef	ENTRADA_FIM_CURSO
#undef	ENTRADA_PRESSOSTATO
#undef	SAIDA_TRAVA_ELEVADOR
#undef	SAIDA_TRAVA_FREIO
