// "$Date: 2018-03-23 13:11:01 -0300 (sex, 23 mar 2018) $"
// "$Author: fares $"
// "$Revision: 4195 $"

#include "IDispositivoMotor.h"
#include <QApplication>

#if defined(_DEBUG)
#  include <QDebug>
#endif

#ifndef	QTSERIALTEST	// definido apenas no arquivo do projeto "QtSerialTest"
#  include "../QtCliche/CDadosConfiguracao.h"
#  define INICIALIZA_PROPERTY(Classe, Nome, Indice) \
	p_##Nome.setContainer(this); \
	p_##Nome.getter(&IDispositivoMotor::pegar##Nome );	\
	p_##Nome.setter(&IDispositivoMotor::slot##Nome##_Mudou ); \
	connect(mDadosConfiguracao, SIGNAL(sinal_##Classe##_##Nome##_##Indice##_alterado(qint32)), this, SLOT(slot##Nome##_Mudou(qint32))); \
	m##Nome = mDadosConfiguracao->p_##Classe##_##Nome##_##Indice;

#  define FINALIZA_PROPERTY(Classe, Nome, Indice) \
	disconnect(mDadosConfiguracao, SIGNAL(sinal_##Classe##_##Nome##_##Indice##_alterado(qint32)), this, SLOT(slot##Nome##_Mudou(qint32)));

#else

#  define INICIALIZA_PROPERTY(Classe, Nome, Indice) \
	p_##Nome.setContainer(this); \
	p_##Nome.getter(&IDispositivoMotor::pegar##Nome );	\
	p_##Nome.setter(&IDispositivoMotor::slot##Nome##_Mudou );

#  define FINALIZA_PROPERTY(Classe, Nome, Indice)

#endif

#if defined(_DETALHE_MOTOR_APPLIED) || defined(_DETALHE_MOTOR_TESTE)
#   include "Erros.h"
#	define LOG_DETALHE(a,b,c)	emit Log(a,b,c);
#	define LOG_DETALHE_CILINDRO(a,b,c)	if (mTipoDispositivo==IDispositivoMotor::TipoCilindro) { emit Log(a,b,c); }
#else
#	define LOG_DETALHE(a,b,c)	{ ; }
#	define LOG_DETALHE_CILINDRO(a,b,c)	{ ; }
#endif
#if defined(_DEBUG)
#   include "Erros.h"
#	define LOG_DEBUG(a,b,c)	emit Log(a,b,c);
#else
#	define LOG_DEBUG(a,b,c)	{ ; }
#endif


#include "../QtCliche/NomesDispositivos.h"
#include <QMetaType>
// #include <QDebug>

#define INTERVALO_TIMEOUT_SERIAL	84	/* trata-se de um mínimo, pois o número de bytes transmitidos e o esperado são adicionados; */
#define INTERVALO_LEITURA_ESTADO	50

IDispositivoMotor::IDispositivoMotor(CDadosConfiguracao * dadosConfiguracao, QObject * pai) : mDadosConfiguracao(dadosConfiguracao), mPai(pai) {
	qRegisterMetaType<IDispositivoMotor::tipo>("IDispositivoMotor::tipo");
	qRegisterMetaType<IDispositivoMotor::sentido>("IDispositivoMotor::sentido");
	qRegisterMetaType<IDispositivoMotor::intensidade>("IDispositivoMotor::intensidade");

	mTerminador = '\r';
	mCaracteresEspeciais.clear();

	mCoeficientePosEncoderParaPosPassos = 1.0;
	mCoeficientePos_uMParaPosPassos = 1.0;

	mModoTeste = false;
	mModoTesteSentido = IDispositivoMotor::Direita;
	mModoTesteTimer.setSingleShot(false);
	mModoTesteTimer.setInterval(1500);
	mModoTesteTimer.stop();
	connect(&mModoTesteTimer, SIGNAL(timeout()), this, SLOT(slotModoTesteTimerTimeout()));

	ajustarTipoDispositivo(IDispositivoMotor::TipoIndefinido);

	// setTerminationEnabled(true);
	mEnderecoDispositivo="?";

#ifndef _DISPOSITIVOS_TESTE
	mPortaSerial = new QextSerialPort();
	mPortaSerial->setBaudRate (BAUD9600);
	mPortaSerial->setDataBits (DATA_8);
	mPortaSerial->setFlowControl (FLOW_OFF);
	mPortaSerial->setParity (PAR_NONE);
	mPortaSerial->setStopBits (STOP_1);

#  ifdef _WIN32
	mPortaSerial->setQueryMode(QextSerialPort::Polling);
	mPortaSerial->setTimeout(2);
#  endif

#  ifdef __linux
	mPortaSerial->setQueryMode(QextSerialPort::Polling);
	mPortaSerial->setTimeout(2);
#  endif
#endif

#ifdef	_WIN32
	mPortaNome="COM1";
#endif
#ifdef	__linux
	mPortaNome="/dev/ttyS0";
#endif
	mPortasDisponiveis.clear();

#ifndef _DISPOSITIVOS_TESTE
	mBufferSaida.clear();
	mBufferEntrada.clear();
#endif
	mMonitorandoPosicaoEncoder = false;
	mMonitorandoPosicaoPassos = false;

#ifndef _DISPOSITIVOS_TESTE
	mTimerTimeOut = new QTimer(this);
	mTimerTimeOut->setSingleShot(true);
	mTimerTimeOut->setInterval(INTERVALO_TIMEOUT_SERIAL);
	mTimerTimeOut->stop();
	connect(mTimerTimeOut, SIGNAL(timeout()), this, SLOT(slotPortaTimeOut()));
#endif

	mPermanecerRodando = false;
	mComandosEmProcessamento = 0;
	mRodando = false;

	mInicializado = false;
	mSinalizadoInicializado = false;

	mContadorFalhaTransmissao = 0;
	mContadorRetentativasTransmissao = 0;
	mRemoverComandoDaFila = false;
	mModeloProduto = "desconhecido";

#ifndef _DISPOSITIVOS_TESTE
	connect(this, SIGNAL(sinalLidoParcialmente()), this, SLOT(slotLidoParcialmente()));
	connect(this, SIGNAL(sinalLidoCompletamente()), this, SLOT(slotLidoCompletamente()));
#endif
	connect(this, SIGNAL(sinalMudancaTipoDetectado(IDispositivoMotor::tipo, IDispositivoMotor::tipo, QString)), this, SLOT(slotMudancaTipoDetectado(IDispositivoMotor::tipo,IDispositivoMotor::tipo,QString)), Qt::QueuedConnection);
	mEmMovimento = false;
	mEmMovimentoAnterior = false;
	mLimiteAtingido = false;

	mRestringindoPortasIndice = 0;
	mRestringindoPortasListaPortas.clear();
	mPosicaoPassos = 0;
	mPosicaoPassosAnterior = 0;
	mPosicaoPassosDesejada = 0;
	mPosicaoEncoder = 0;
	mPosicaoEncoderAnterior = 0;	// anterior, de antes de começar o movimento
	mPosicaoEncoderAntiga = 0;		// antiga, da última posição conhecida antes da atual
	mPosicaoEncoderDesejada = 0;
	mPosicaoEncoderDiferenca = 0;
	mEstados = 0;
	mEstadosAnteriores = 7;
	mAlarmes = 0;
	mAlarmesAnteriores = 7;
	mSaidas = 0;
	mSaidasAnteriores = 7;
	mEntradas = 0;
	mEntradasAnteriores = 7;
#if defined(_DEBUG)
	qDebug() << "IDispositivoMotor: comecei.";
#endif
}

void IDispositivoMotor::procurarPortas() {
	QString auxNomePorta;
	for (int i = 0; i < 16; i++) {
#ifdef _WIN32
		auxNomePorta = QString("COM%1").arg(i+1);
#endif
#ifdef __linux

		auxNomePorta = QString("/dev/ttyS%1").arg(i);
		mPortaSerial->setPortName(auxNomePorta);
		if ( mPortaSerial->open(QextSerialPort::ReadWrite) ) {
			mPortasDisponiveis.append(auxNomePorta);
			mPortaSerial->close();
		}

		auxNomePorta = QString("/dev/ttyUSB%1").arg(i);
#endif
		mPortaSerial->setPortName(auxNomePorta);
		if ( mPortaSerial->open(QextSerialPort::ReadWrite) ) {
			mPortasDisponiveis.append(auxNomePorta);
			mPortaSerial->close();
		}
	}
	mPortasDisponiveis.sort();
}

IDispositivoMotor::~IDispositivoMotor() {
#ifndef _DISPOSITIVOS_TESTE
	mModoTesteTimer.stop();
	disconnect(&mModoTesteTimer, SIGNAL(timeout()), this, SLOT(slotModoTesteTimerTimeout()));

	disconnect(this, SIGNAL(sinalLidoParcialmente()), this, SLOT(slotLidoParcialmente()));
	disconnect(this, SIGNAL(sinalLidoCompletamente()), this, SLOT(slotLidoCompletamente()));

	if (mTimerTimeOut->isActive()) {
		mTimerTimeOut->stop();
	}
	disconnect(mTimerTimeOut, SIGNAL(timeout()), this, SLOT(slotPortaTimeOut()));

	mPortaSerial->close();

	delete mTimerTimeOut;
	delete mPortaSerial;
#endif
	disconnect(this, SIGNAL(sinalMudancaTipoDetectado(IDispositivoMotor::tipo, IDispositivoMotor::tipo, QString)), this, SLOT(slotMudancaTipoDetectado(IDispositivoMotor::tipo,IDispositivoMotor::tipo,QString)));
}

QString IDispositivoMotor::ajustaValor(qint32 numero, int decimais) {
	QString texto = QString::number(numero);
	QChar inicial = QChar(' ');
	if (texto.at(0) == QChar('-')) {
		inicial = QChar('-');
		texto.remove(0,1);
	}
	if (texto.size() > decimais) {
		if (decimais > 0) {
			texto.insert(texto.size() - decimais, '.');
		}
	} else {
		while (texto.size() < decimais) {
			texto.insert(0,"0");
		}
		texto.insert(0, '.');
		texto.insert(0, "0");
	}
	if (inicial == '-') {
		texto.insert(0, inicial);
	}
	return texto;
}

bool IDispositivoMotor::abrirPorta(QString nomePorta) {
	bool resultado = true;
	if (QString("") == nomePorta) {
		nomePorta = mPortaNome;
	}
#ifdef _DISPOSITIVOS_TESTE
	mPortaNome = nomePorta;
#else
	LOG_DEBUG(OBSERVACAO, "IDispositivoMotor::abrirPorta", QString("%1: tentando abrir porta: ").arg(mNomeDispositivo), nomePorta);
	mSemErroComunicacao = false;
	mPortaSerial->setPortName(nomePorta);
	if ( ( resultado = mPortaSerial->open(QextSerialPort::ReadWrite) ) ) {
		mPortaNome = nomePorta;
		mFilaComandos.clear();
		mComandosEmProcessamento = 0;
		mIndiceMensagem = 0;
		mBufferSaida.clear();
		mBufferEntrada.clear();
		// mPortaSerial->reset();
		mPortaSerial->flush();
		mPortaSerial->setTextModeEnabled(false);
		// mModeloProduto = "desconhecido";
		mPermanecerRodando = true;
		// mTipoDispositivo = IDispositivoMotor::TipoIndefinido;
		if (mAlarmesEstados.size() > 0) {
			for(int i=0; i < mAlarmesEstados.size(); i++) {
				mAlarmesEstados.data()[i] = false;
			}
		}
		if (mEstadosEstados.size() > 0) {
			for(int i=0; i < mEstadosEstados.size(); i++) {
				mEstadosEstados.data()[i] = false;
			}
		}
		if (mSaidasEstados.size() > 0) {
			for(int i=0; i < mSaidasEstados.size(); i++) {
				mSaidasEstados.data()[i] = false;
			}
		}
		if (mEntradasEstados.size() > 0) {
			for(int i=0; i < mEntradasEstados.size(); i++) {
				mEntradasEstados.data()[i] = false;
			}
		}

		// apenas para provocar uma diferença, de modo que sejam sinalizados na proxima rodada de busca de estados
		mAlarmesAnteriores  = mAlarmes  + 7;
		mEstadosAnteriores  = mEstados  + 7;
		mSaidasAnteriores   = mSaidas   + 7;
		mEntradasAnteriores = mEntradas + 7;

		start();

		LOG_DETALHE_CILINDRO(OBSERVACAO, "IDispositivoMotor::abrirPorta", QString("%1: sucesso ao abrir a porta.").arg(mNomeDispositivo));
	} else {
		resultado = false;
		LOG_DETALHE_CILINDRO(ERRO, "IDispositivoMotor::abrirPorta", QString("%1: ERRO AO ABRIR A PORTA.").arg(mNomeDispositivo));
	}
#endif
	return resultado;
}

void IDispositivoMotor::fecharPorta(bool obriga) {
	mInicializado = false;
#ifdef _DISPOSITIVOS_TESTE
	Q_UNUSED(obriga);
#else
	if ( ! obriga ) {
		LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::fecharPorta", QString("%1: aguardando para fechar a porta: ").arg(mNomeDispositivo), mPortaSerial->portName());
		while ( ( ! mFilaComandos.isEmpty() ) || ( mComandosEmProcessamento > 0 ) || ( mPortaSerial->bytesToWrite() > 0 ) || ( mPortaSerial->bytesAvailable() > 0 ) ) {
			qApp->processEvents();
			yieldCurrentThread();
			// LOG_DETALHE(ADVERTENCIA, "IDispositivoMotor::fecharPorta", QString("comandos a serem processados: %1").arg(mComandosEmProcessamento));
			usleep(100);
		}
	}
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::fecharPorta", QString("%1: fechando a porta. ").arg(mNomeDispositivo), mPortaSerial->portName());
	mPermanecerRodando = false;
	mSinalizadoInicializado = false;
	mRodando = false;
	if (isRunning()) {
		if ( ! wait(100) ) {
			terminate();
			wait();
		}
	}
	if (mPortaSerial->isOpen()) {
		if (mTimerTimeOut->isActive()) {
			mTimerTimeOut->stop();
		}
		mPortaSerial->close();
	}
	// mModeloProduto = "desconhecido";
#endif
}

void IDispositivoMotor::ajustarVelocidadePorta(int velocidade) {
	switch (velocidade) {
		case 110:
			mPortaSerial->setBaudRate(BAUD110);
			break;
		case 300:
			mPortaSerial->setBaudRate(BAUD300);
			break;
		case 600:
			mPortaSerial->setBaudRate(BAUD600);
			break;
		case 1200:
			mPortaSerial->setBaudRate(BAUD1200);
			break;
		case 2400:
			mPortaSerial->setBaudRate(BAUD2400);
			break;
		case 4800:
			mPortaSerial->setBaudRate(BAUD4800);
			break;
		case 9600:
			mPortaSerial->setBaudRate(BAUD9600);
			break;
		case 19200:
			mPortaSerial->setBaudRate(BAUD19200);
			break;
		case 38400:
			mPortaSerial->setBaudRate(BAUD38400);
			break;
		case 57600:
			mPortaSerial->setBaudRate(BAUD57600);
			break;
		case 115200:
			mPortaSerial->setBaudRate(BAUD115200);
			break;
#if defined(_DEBUG) || defined(_DETALHE_QTCLICHE) || defined(_DETALHE_MODELODADOS) || defined(_DETALHE_MOTOR_APPLIED) || defined(_DETALHE_MOTOR_TESTE)
		default:
			mPortaSerial->setBaudRate(BAUD9600);
#endif
	}
}

void IDispositivoMotor::enviaComando(QString comando, bool prioritario, bool especialUsuario) {  // o parâmetro "prioritario" tem um sentido de urgência também;
	LOG_DETALHE(DEPURACAO, "IDispositivoMotor::enviaComando", QString("%1: inserido comando na fila: %2").arg(mNomeDispositivo).arg(comando.simplified()));
	mMutexEnviaComando.lock();
	if (prioritario) {
		mFilaComandos.insert(0, CComando(comando.toLatin1(), false, 0, especialUsuario));
	} else {
		mFilaComandos.append(CComando(comando.toLatin1(), false, 0, especialUsuario));
	}
	mMutexEnviaComando.unlock();
	if ( ! mRodando ) {
		mRodando = true;
		enviaComandoFila();
	}
}

void IDispositivoMotor::enviaComandoHex(QString comandoHex, bool usaSeparador, int tamanhoRespostaEsperada) {
	foreach (QString comando, comandoHex.split(' ')) {
		QByteArray prov = comando.toLatin1();
		mBufferComando.append(QByteArray::fromHex(prov));
	}
	if ( usaSeparador ) {
		mBufferComando.append(SEPARADOR_COMUNICACAO);
	}
	mBufferComando.append('\x00');  // finalizador de "string"

	LOG_DETALHE(DEPURACAO, "IDispositivoMotor::enviaComandoHex", QString("%1: inserido comando na fila: %2").arg(mNomeDispositivo).arg(QString(mBufferComando.toHex())));
	mMutexEnviaComando.lock();
	mFilaComandos.append(CComando(mBufferComando,true,tamanhoRespostaEsperada, false));
	mMutexEnviaComando.unlock();
	if ( ! mRodando ) {
		mRodando = true;
		enviaComandoFila();
	}
	mBufferComando.clear();
}

//! \todo Testar a comparação aternativa "if ( mComandosEmProcessamento < mFilaComandos.size() )" e verificar o "mRodando = false"
void IDispositivoMotor::enviaComandoFila() {
	mMutexEnviaComandoFila.lock();
	if (mFilaComandos.size() > 0) {
		if ( mComandosEmProcessamento == 0 ) {		// if ( mComandosEmProcessamento < mFilaComandos.size() ) {
			mBufferSaida = mFilaComandos.first().mComando;
			mBufferSaidaAntigo = mBufferSaida;
			mComandosEmProcessamento ++;
			LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::enviaComandoFila", QString("%1: comandos a serem processados: %2").arg(mNomeDispositivo).arg(mComandosEmProcessamento));
			mEstadoComandoTotalmenteTransmitido = ( mPortaSerial->write(mBufferSaida.constData(), mBufferSaida.size() ) == mBufferSaida.size() );
			mValorTempoTimeOut = INTERVALO_TIMEOUT_SERIAL + mFilaComandos.first().mTamanhoRespostaEsperada + mBufferSaida.size() * 2;
			mTimerTimeOut->setInterval(mValorTempoTimeOut);	// o primeiro byte pode demorar um pouco mais a chegar, o resto é que vem rápido.
			mTimerTimeOut->start();
		} else {
			mRodando = false;
		}
	}
	mMutexEnviaComandoFila.unlock();
	// LOG_DETALHE(ADVERTENCIA, "IDispositivoMotor::enviaComandoFila", QString("%1: limite de tempo ajustado para: %2").arg(mNomeDispositivo).arg(mValorTempoTimeOut));
	LOG_DETALHE(DEPURACAO, "IDispositivoMotor::enviaComandoFila", QString("%1: comando enviado: %2: %3").arg(mNomeDispositivo).arg(mEstadoComandoTotalmenteTransmitido).arg(QString(mFilaComandos.first().mBinario ? mBufferSaida.toHex() : mBufferSaida.simplified())));
}

void IDispositivoMotor::configurarTipo(IDispositivoMotor::tipo tipo, QString porta) {
#ifndef _DISPOSITIVOS_TESTE
	if (porta == "") {
		porta = mPortaSerial->portName();
		LOG_DETALHE(DEPURACAO, "IDispositivoMotor::configurarTipo", QString("%1: usando porta em uso atualmente: %2").arg(mNomeDispositivo).arg(porta));
	}
#endif
	mPortaNome = porta;
	mTipoDispositivo = tipo;
	mNomeDispositivo = tipoEmNome(tipo);
	switch (tipo) {
		case IDispositivoMotor::TipoCamera1:
			INICIALIZA_PROPERTY(ConjuntoCamera, Aceleracao, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, CorrenteMovimento, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, CorrenteParado, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DefasagemCentroLente, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DistanciaReferenciaRelativa, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DividendoEncoder_uM, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DivisorEncoder_uM, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DivisorMotorEncoder, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DividendoMotorEncoder, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MargemMinimaNegativa, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MargemMinimaPositiva, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoGrande, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoMedio, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoPequeno, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, PosicaoReferenciaAbsoluta, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, ResolucaoMotor, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, SensorAtivado, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeAlta, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeBaixa, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMaxima, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMedia, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMinima, 1)
			break;
		case IDispositivoMotor::TipoCamera2:
			INICIALIZA_PROPERTY(ConjuntoCamera, Aceleracao, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, CorrenteMovimento, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, CorrenteParado, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, DefasagemCentroLente, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, DistanciaReferenciaRelativa, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, DividendoEncoder_uM, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, DivisorEncoder_uM, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, DivisorMotorEncoder, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, DividendoMotorEncoder, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, MargemMinimaNegativa, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, MargemMinimaPositiva, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoGrande, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoMedio, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoPequeno, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, PosicaoReferenciaAbsoluta, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, ResolucaoMotor, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, SensorAtivado, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeAlta, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeBaixa, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMaxima, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMedia, 2)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMinima, 2)
			break;
		case IDispositivoMotor::TipoCilindro:
			INICIALIZA_PROPERTY(ConjuntoCilindro, Aceleracao, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, CorrenteMovimento, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, CorrenteParado, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, DefasagemCentroLente, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, DistanciaReferenciaRelativa, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, DividendoEncoder_uM, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, DivisorEncoder_uM, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, DivisorMotorEncoder, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, DividendoMotorEncoder, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, MargemMinimaNegativa, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, MargemMinimaPositiva, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, MovimentoGrande, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, MovimentoMedio, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, MovimentoPequeno, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, PosicaoReferenciaAbsoluta, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, ResolucaoMotor, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, SensorAtivado, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, VelocidadeAlta, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, VelocidadeBaixa, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, VelocidadeMaxima, 1)
			INICIALIZA_PROPERTY(ConjuntoCilindro, VelocidadeMedia, 1)
			// INICIALIZA_PROPERTY(ConjuntoCilindro, VelocidadeMinima, 1)
			break;
		case IDispositivoMotor::TipoIndefinido:
			INICIALIZA_PROPERTY(ConjuntoCamera, Aceleracao, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, CorrenteMovimento, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, CorrenteParado, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DefasagemCentroLente, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DistanciaReferenciaRelativa, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DividendoEncoder_uM, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DivisorEncoder_uM, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DivisorMotorEncoder, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, DividendoMotorEncoder, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MargemMinimaNegativa, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MargemMinimaPositiva, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoGrande, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoMedio, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, MovimentoPequeno, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, PosicaoReferenciaAbsoluta, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, ResolucaoMotor, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, SensorAtivado, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeAlta, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeBaixa, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMaxima, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMedia, 1)
			INICIALIZA_PROPERTY(ConjuntoCamera, VelocidadeMinima, 1)
			break;
		default:
			break;
	}
}

#ifndef	QTSERIALTEST	// definido apenas no arquivo do projeto "QtSerialTest"
void IDispositivoMotor::desconectarSinaisConfiguracao() {
	switch (mTipoDispositivo) {
		case IDispositivoMotor::TipoCamera1:
			FINALIZA_PROPERTY(ConjuntoCamera, Aceleracao, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, CorrenteMovimento, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, CorrenteParado, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DefasagemCentroLente, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DistanciaReferenciaRelativa, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DividendoEncoder_uM, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DivisorEncoder_uM, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DivisorMotorEncoder, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DividendoMotorEncoder, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MargemMinimaNegativa, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MargemMinimaPositiva, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoGrande, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoMedio, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoPequeno, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, PosicaoReferenciaAbsoluta, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, ResolucaoMotor, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, SensorAtivado, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeAlta, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeBaixa, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMedia, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMaxima, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMinima, 1)
			break;
		case IDispositivoMotor::TipoCamera2:
			FINALIZA_PROPERTY(ConjuntoCamera, Aceleracao, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, CorrenteMovimento, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, CorrenteParado, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, DefasagemCentroLente, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, DistanciaReferenciaRelativa, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, DividendoEncoder_uM, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, DivisorEncoder_uM, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, DivisorMotorEncoder, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, DividendoMotorEncoder, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, MargemMinimaNegativa, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, MargemMinimaPositiva, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoGrande, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoMedio, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoPequeno, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, PosicaoReferenciaAbsoluta, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, ResolucaoMotor, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, SensorAtivado, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeAlta, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeBaixa, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMedia, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMaxima, 2)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMinima, 2)
			break;
		case IDispositivoMotor::TipoCilindro:
			FINALIZA_PROPERTY(ConjuntoCilindro, Aceleracao, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, CorrenteMovimento, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, CorrenteParado, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, DefasagemCentroLente, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, DistanciaReferenciaRelativa, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, DividendoEncoder_uM, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, DivisorEncoder_uM, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, DivisorMotorEncoder, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, DividendoMotorEncoder, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, MargemMinimaNegativa, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, MargemMinimaPositiva, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, MovimentoGrande, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, MovimentoMedio, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, MovimentoPequeno, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, PosicaoReferenciaAbsoluta, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, ResolucaoMotor, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, SensorAtivado, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, VelocidadeAlta, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, VelocidadeBaixa, 1)
			FINALIZA_PROPERTY(ConjuntoCilindro, VelocidadeMedia, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, VelocidadeMaxima, 1)
			// FINALIZA_PROPERTY(ConjuntoCilindro, VelocidadeMinima, 1)
			break;
		case IDispositivoMotor::TipoIndefinido:
			FINALIZA_PROPERTY(ConjuntoCamera, Aceleracao, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, CorrenteMovimento, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, CorrenteParado, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DefasagemCentroLente, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DistanciaReferenciaRelativa, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DividendoEncoder_uM, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DivisorEncoder_uM, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DivisorMotorEncoder, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, DividendoMotorEncoder, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MargemMinimaNegativa, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MargemMinimaPositiva, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoGrande, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoMedio, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, MovimentoPequeno, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, PosicaoReferenciaAbsoluta, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, ResolucaoMotor, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, SensorAtivado, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeAlta, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeBaixa, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMedia, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMaxima, 1)
			FINALIZA_PROPERTY(ConjuntoCamera, VelocidadeMinima, 1)
			break;
#if defined(_DEBUG) || defined(_DETALHE_QTCLICHE) || defined(_DETALHE_MODELODADOS) || defined(_DETALHE_MOTOR_APPLIED) || defined(_DETALHE_MOTOR_TESTE)
		default:
			break;
#endif
	}
}
#endif

void IDispositivoMotor::run() {
	LOG_DETALHE_CILINDRO(ADVERTENCIA, "IDispositivoMotor::run", QString("%1: iniciando processamento paralelo.").arg(mNomeDispositivo));
	while (mPermanecerRodando ) {
		mMutex.lock();
		if ( ( mPortaSerial->bytesAvailable() > 0 ) && ( mFilaComandos.size() > 0 ) ) {
			mBufferEntrada.append(mPortaSerial->readAll());
			if (mFilaComandos.first().mBinario && ( mFilaComandos.first().mTamanhoRespostaEsperada > 0 ) ) {
				if (mBufferEntrada.size() < mFilaComandos.first().mTamanhoRespostaEsperada) {
					LOG_DETALHE(DEPURACAO, "IDispositivoMotor::run", QString("%1: lido parcialmente: %2").arg(mNomeDispositivo).arg(QString(mBufferEntrada.toHex())));
					emit sinalLidoParcialmente();
				} else {
					mBufferRecebido = mBufferEntrada.left(mFilaComandos.first().mTamanhoRespostaEsperada);
					mBufferEntrada.remove(0,mFilaComandos.first().mTamanhoRespostaEsperada);
					emit sinalLidoCompletamente();
					LOG_DETALHE(INFORMACAO, "IDispositivoMotor::run", QString("%1: lido completamente: %2").arg(mNomeDispositivo).arg(QString(mBufferRecebido.toHex())));
				}
			} else {
				// LOG_DETALHE(DEPURACAO, "IDispositivoMotor::run", QString("%1: lido parcialmente: %2").arg(mNomeDispositivo), mBufferEntrada.simplified());
				emit sinalLidoParcialmente();
				if (mBufferEntrada.contains(mTerminador)) {
					for (int i = mBufferEntrada.size() - 1; i >= 0 ; i--) {
						if ( ( (mBufferEntrada.at(i) < 0x20) || ( mBufferEntrada.at(i) > 0x5F ) ) && ( ! mCaracteresEspeciais.contains(mBufferEntrada.at(i)) )  ) {
							mBufferEntrada.remove(i,1);
						}
					}
					int posicao = mBufferEntrada.indexOf(mTerminador);
					while (posicao > 0) {	// no caso de haver recebido mais do que uma resposta de uma vez só;
						mBufferRecebido = mBufferEntrada.left(posicao + 1);
						mBufferEntrada.remove(0,posicao + 1);
						emit sinalLidoCompletamente();
						posicao = mBufferEntrada.indexOf(mTerminador);
						LOG_DETALHE(INFORMACAO, "IDispositivoMotor::run", QString("%1: lido completamente: %2").arg(mNomeDispositivo).arg(QString(mBufferRecebido.simplified())));
					}
					usleep(400);
				}
			}
			usleep(100);
// #if defined(_DETALHE_MOTOR_APPLIED)
// 		} else {
// 			if ( ( mPortaSerial->bytesAvailable() > 0 ) && ( mFilaComandos.size() == 0 ) ) {
// 				qDebug() << "fila de comandos vazia!!!";
// 			}
// #endif
		}
		mMutex.unlock();
		// LOG_DETALHE(DEPURACAO, "IDispositivoMotor::run", QString("%1: reciclando processamento paralelo.").arg(mNomeDispositivo));
	}
	LOG_DETALHE_CILINDRO(ADVERTENCIA, "IDispositivoMotor::run", QString("%1: finalizando processamento paralelo.").arg(mNomeDispositivo));
}

void IDispositivoMotor::slotPortaTimeOut() {
	mBufferEntrada.clear();
	mRodando = false;
	if (mFilaComandos.first().mBinario) {
		LOG_DETALHE(ADVERTENCIA, "IDispositivoMotor::slotPortaTimeOut",QString( "--- %1: 'timeout' de recebimento: %2").arg(mNomeDispositivo).arg(QString(mBufferSaida.toHex())));
		mRemoverComandoDaFila = true;
		removerComandoTransmitido(mRemoverComandoDaFila);
	} else {
		LOG_DETALHE(ADVERTENCIA, "IDispositivoMotor::slotPortaTimeOut", QString( "--- %1: 'timeout' de recebimento: %2").arg(mNomeDispositivo).arg(QString(mBufferSaida.simplified())));
		// comando = mFilaComandos.at(0).mComando;
		if ( mInicializado && (mRestringindoPortasIndice == 0) ) {
			mContadorFalhaTransmissao += 1;
			mRemoverComandoDaFila = false;
			if ( mContadorFalhaTransmissao > 2 ) {
				mRemoverComandoDaFila = true;
				LOG_DETALHE(ALERTA, "IDispositivoMotor::slotPortaTimeOut", QString("%1: comando enviado três vezes, sem resposta: %2").arg(mNomeDispositivo).arg(QString(mBufferSaida.simplified())));
				mContadorFalhaTransmissao = 0;
				mContadorRetentativasTransmissao += 1;
				if (mContadorRetentativasTransmissao == 4) {
					mContadorRetentativasTransmissao = 0;
					erroGrave();
				}
			}
		} else {
			mRemoverComandoDaFila = true;
		}
		removerComandoTransmitido(mRemoverComandoDaFila);
	}

	if (mFilaComandos.size() > 0) {	// acaba de ser removido um item, portanto precisa testar de novo o que fazer;
		mRodando = true;
		enviaComandoFila();
	} else {
		LOG_DETALHE(DEPURACAO, "IDispositivoMotor::slotPortaTimeOut", QString("%1: fila vazia no dispositivo na porta.").arg(mNomeDispositivo));
		// mTimerTimeOut->stop();
		mRodando = false;
		if (mInicializado) {
			enviaSolicitacaoEstado();
		}
	}
}

void IDispositivoMotor::erroGrave() {
	LOG_DETALHE(ERRO, "IDispositivoMotor::erroGrave", QString("%1: mBufferEntrada: %2 - mBufferSaida: %3").arg(mNomeDispositivo).arg(QString(mFilaComandos.first().mBinario ? mBufferEntrada.toHex() : mBufferEntrada.simplified())).arg(QString(mFilaComandos.first().mBinario ? mBufferSaida.toHex() : mBufferSaida.simplified())) );
	mSemErroComunicacao = false;
	emit sinalEstadoComunicacao(mTipoDispositivo, false);	// com problema de comunicação
	fecharPorta(true);
	QTimer::singleShot(200, this, SLOT(slotReabrirPorta()));
}

void IDispositivoMotor::slotLidoCompletamente() {
	// LOG_DETALHE(DEPURACAO, "IDispositivoMotor::slotLidoCompletamente", QString("%1: recebimento completo: ").arg(mNomeDispositivo), mBufferEntrada.simplified());
	tratarResposta();
	mRemoverComandoDaFila = true;
	if ( ( ! mComandoReconhecido) && (mEnderecoDispositivo != "?") && ( mBufferSaida == mBufferSaidaAntigo) ) {
		mContadorFalhaTransmissao += 1;
		mRemoverComandoDaFila = false;
		if ( mContadorFalhaTransmissao > 2 ) {
			mRemoverComandoDaFila = true;
			LOG_DETALHE(ADVERTENCIA, "CDispositivoMotorApplied::slotLidoCompletamente", QString("%1: comando enviado três vezes, sem resposta: %2").arg(mNomeDispositivo).arg(QString(mFilaComandos.first().mBinario ? mBufferSaida.toHex() : mBufferSaida.simplified())));
			mContadorFalhaTransmissao = 0;
			mContadorRetentativasTransmissao += 1;
			if (mContadorRetentativasTransmissao == 4) {
				mContadorRetentativasTransmissao = 0;
				erroGrave();
			}
		}
	} else {
		if ( ! mSemErroComunicacao) {
			mSemErroComunicacao = true;
			emit sinalEstadoComunicacao(mTipoDispositivo, true);	// resolveu o problema com comunicação
		}
		mContadorFalhaTransmissao = 0;
		mContadorRetentativasTransmissao = 0;
	}
	if (mRemoverComandoDaFila) {
		removerComandoTransmitido(mEstadoComandoTotalmenteTransmitido);
	}


	// mBufferSaida.clear();
	if (mFilaComandos.size() > 0) {
		if ( ! mSinalizadoInicializado ) {
			if ( (mEnderecoDispositivo != "?") && ( mModeloProduto != "desconhecido" ) ) {
				mSinalizadoInicializado = true;
				emit sinalInicializado(mTipoDispositivo);
			}
		}
		enviaComandoFila();
	} else {
		// LOG_DETALHE(DEPURACAO, "IDispositivoMotor::slotLidoCompletamente", QString("%1: fila vazia no dispositivo na porta.").arg(mNomeDispositivo));
		mTimerTimeOut->stop();
		mRodando = false;
		if (mInicializado) {
			enviaSolicitacaoEstado();
		}
		mBufferEntrada.clear();
	}
}

void IDispositivoMotor::slotLidoParcialmente() {
	mTimerTimeOut->start();		// para evitar o "time-out", já que está recebendo algo;
	// LOG_DETALHE(DEPURACAO, "IDispositivoMotor::slotLidoParcialmente", QString("%1: recebimento parcial.").arg(mNomeDispositivo));
}

void IDispositivoMotor::removerComandoTransmitido(bool remove) {
	if (mFilaComandos.size() > 0) {
#if defined(_DETALHE_MOTOR_APPLIED)
		bool binario = mFilaComandos.first().mBinario;
		QByteArray comando = mFilaComandos.first().mComando;
#endif
		if (remove) {
			mFilaComandos.removeFirst();
			mIndiceMensagem=0;
#if defined(_DETALHE_MOTOR_APPLIED)
			emit Log(DEPURACAO, "IDispositivoMotor::removerComandoTransmitido", QString("%1: comando removido da fila: %2").arg(mNomeDispositivo).arg(QString(binario ? comando.toHex() : comando.simplified())));
		} else {
			emit Log(ADVERTENCIA, "IDispositivoMotor::removerComandoTransmitido", QString("%1: comando não removido da fila: %2").arg(mNomeDispositivo).arg(QString(binario ? comando.toHex() : comando.simplified())));
#endif
		}
	}
	mComandosEmProcessamento --;
	LOG_DETALHE(ADVERTENCIA, "IDispositivoMotor::slotLidoCompletamente", QString("%1: comandos a serem processados: %2").arg(mNomeDispositivo).arg(mComandosEmProcessamento));
}

bool IDispositivoMotor::valorEstado(int ordem, QString * mensagem) {
	if (mensagem != nullptr) {
		*mensagem = mEstadosMensagens.at(ordem);
	}
	return mEstadosEstados.at(ordem);
}

bool IDispositivoMotor::valorAlarme(int ordem, QString * mensagem) {
	if (mensagem != nullptr) {
		*mensagem = mAlarmesMensagens.at(ordem);
	}
	return mAlarmesEstados.at(ordem);
}

bool IDispositivoMotor::valorEntrada(int ordem, QString *mensagem) {
	if (mensagem != nullptr) {
		*mensagem = mEntradasMensagens[mTipoDispositivo].at(ordem);
	}
	return mEntradasEstados.at(ordem);
}

bool IDispositivoMotor::valorSaida(int ordem, QString *mensagem) {
	if (mensagem != nullptr) {
		*mensagem = mSaidasMensagens[mTipoDispositivo].at(ordem);
	}
	return mSaidasEstados.at(ordem);
}

QString IDispositivoMotor::tipoEmNome(IDispositivoMotor::tipo tipo) {
	QString resposta;
	switch (tipo) {
		case IDispositivoMotor::TipoCamera1:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM1;
			break;
		case IDispositivoMotor::TipoCamera2:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM2;
			break;
		case IDispositivoMotor::TipoCamera3:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM3;
			break;
		case IDispositivoMotor::TipoCamera4:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM4;
			break;
		case IDispositivoMotor::TipoCamera5:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM5;
			break;
		case IDispositivoMotor::TipoCamera6:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM6;
			break;
		case IDispositivoMotor::TipoCamera7:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM7;
			break;
		case IDispositivoMotor::TipoCamera8:
			resposta = NOME_ACESSO_DISPOSITIVO_CAM8;
			break;
		case IDispositivoMotor::TipoCilindro:
			resposta = NOME_ACESSO_DISPOSITIVO_CILI;
			break;
		case IDispositivoMotor::TipoIndefinido:
			resposta = NOME_ACESSO_DISPOSITIVO_NOVO;
			break;
		default:
			resposta = NOME_ACESSO_DISPOSITIVO_NOVO;
	}
	return resposta;
}

bool IDispositivoMotor::portaAberta() {
#ifdef _DISPOSITIVOS_TESTE
	return true;
#else
	return mPortaSerial->isOpen();
#endif
}


QString IDispositivoMotor::nomePorta() {
#ifdef _DISPOSITIVOS_TESTE
	return QString("Ok");
#else
	return mPortaSerial->portName();
#endif
}

void IDispositivoMotor::slotAceleracao_Mudou(qint32 valor) {
	mAceleracao = valor;
	if (mInicializado && (mEnderecoDispositivo != "?") && (mTipoDispositivo != IDispositivoMotor::TipoIndefinido) ) {
		efetivarConfiguracao();
	}
}

void IDispositivoMotor::slotCorrenteMovimento_Mudou(qint32 valor) {
	mCorrenteMovimento = valor;
	if (mInicializado && (mEnderecoDispositivo != "?") && (mTipoDispositivo != IDispositivoMotor::TipoIndefinido) ) {
		efetivarConfiguracao();
	}
}

void IDispositivoMotor::slotCorrenteParado_Mudou(qint32 valor) {
	mCorrenteParado = valor;
	if (mInicializado && (mEnderecoDispositivo != "?") && (mTipoDispositivo != IDispositivoMotor::TipoIndefinido) ) {
		efetivarConfiguracao();
	}
}

void IDispositivoMotor::slotResolucaoMotor_Mudou(qint32 valor) {
	mResolucaoMotor = valor;
	if (mInicializado && (mEnderecoDispositivo != "?") && (mTipoDispositivo != IDispositivoMotor::TipoIndefinido) ) {
		efetivarConfiguracao();
	}
}

void IDispositivoMotor::slotSensorAtivado_Mudou(qint32 valor) {
	mSensorAtivado = valor;
	if (mInicializado && (mEnderecoDispositivo != "?") && (mTipoDispositivo != IDispositivoMotor::TipoIndefinido) ) {
		efetivarConfiguracao();
	}
}

void IDispositivoMotor::slotDefasagemCentroLente_Mudou(qint32 valor) {
	mDefasagemCentroLente = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotDistanciaReferenciaRelativa", QString(" %1: valor de DistanciaReferenciaRelativa ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotDistanciaReferenciaRelativa_Mudou(qint32 valor) {
	mDistanciaReferenciaRelativa = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotDistanciaReferenciaRelativa", QString("%1: valor de DistanciaReferenciaRelativa ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotDividendoEncoder_uM_Mudou(qint32 valor) {
	mDividendoEncoder_uM = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotDividendoEncoder_uM", QString("%1: valor de DividendoEncoder_uM ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotDivisorEncoder_uM_Mudou(qint32 valor) {
	mDivisorEncoder_uM = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotDivisorEncoder_uM", QString(" %1: valor de DivisorEncoder_uM ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotDivisorMotorEncoder_Mudou(qint32 valor) {
	mDivisorMotorEncoder = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotDivisorMotorEncoder", QString("%1: valor de DivisorMotorEncoder ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotDividendoMotorEncoder_Mudou(qint32 valor) {
	mDividendoMotorEncoder = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotDividendoMotorEncoder", QString("%1: valor de DividendoMotorEncoder ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotMargemMinimaNegativa_Mudou(qint32 valor) {
	mMargemMinimaNegativa = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotMargemMinimaNegativa", QString("%1: valor de MargemMinimaNegativa ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotMargemMinimaPositiva_Mudou(qint32 valor) {
	mMargemMinimaPositiva = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotMargemMinimaPositiva", QString("%1: valor de MargemMinimaPositiva ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotMovimentoGrande_Mudou(qint32 valor) {
	mMovimentoGrande = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotMovimentoGrande", QString("%1: valor de MovimentoGrande ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotMovimentoMedio_Mudou(qint32 valor) {
	mMovimentoMedio = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotMovimentoMedio", QString("%1: valor de MovimentoMedio ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotMovimentoPequeno_Mudou(qint32 valor) {
	mMovimentoPequeno = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotMovimentoPequeno", QString("%1: valor de MovimentoPequeno ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotPosicaoReferenciaAbsoluta_Mudou(qint32 valor) {
	mPosicaoReferenciaAbsoluta = valor;
	LOG_DETALHE_CILINDRO(OBSERVACAO, "IDispositivoMotor::slotPosicaoReferenciaAbsoluta", QString("%1: valor de PosicaoReferenciaAbsoluta ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotVelocidadeAlta_Mudou(qint32 valor) {
	mVelocidadeAlta = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotVelocidadeAlta", QString("%1: valor de VelocidadeAlta ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotVelocidadeBaixa_Mudou(qint32 valor) {
	mVelocidadeBaixa = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotVelocidadeBaixa", QString("%1: valor de VelocidadeBaixa ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotVelocidadeMedia_Mudou(qint32 valor) {
	mVelocidadeMedia = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotVelocidadeMedia", QString("%1: valor de VelocidadeMedia ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotVelocidadeMaxima_Mudou(qint32 valor) {
	mVelocidadeMaxima = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotVelocidadeMaxima", QString("%1: valor de VelocidadeMaxima ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

void IDispositivoMotor::slotVelocidadeMinima_Mudou(qint32 valor) {
	mVelocidadeMinima = valor;
	LOG_DETALHE(OBSERVACAO, "IDispositivoMotor::slotVelocidadeMinima", QString("%1: valor de VelocidadeMinima ajustado para %2").arg(mNomeDispositivo).arg(valor));
}

#ifdef LOG_DETALHE_CILINDRO
#undef LOG_DETALHE_CILINDRO
#endif
#ifdef LOG_DETALHE
#undef LOG_DETALHE
#endif
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif

#ifdef INICIALIZA_PROPERTY
#undef INICIALIZA_PROPERTY
#endif
#ifdef FINALIZA_PROPERTY
#undef FINALIZA_PROPERTY
#endif
