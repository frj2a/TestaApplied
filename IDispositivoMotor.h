// "$Date: 2018-03-23 13:11:01 -0300 (sex, 23 mar 2018) $"
// "$Author: fares $"
// "$Revision: 4195 $"

#ifndef IMOTOR_H
#define IMOTOR_H

#include "../QtCliche/Property.h"
#include <QMap>
#include <QList>
#include <QTimer>
#include <QThread>
#include <QVector>
#include <QSemaphore>
#include <QStringList>
#include <QMutexLocker>
#include <qextserialport.h>	// se pressupõe que sempre trabalharemos com portas seriais RS-232

#define SEPARADOR_COMUNICACAO '\x0D'

class CDadosConfiguracao;

class CComando {
public:
	QByteArray	mComando;
	bool		mBinario;
	int			mTamanhoRespostaEsperada;
	bool		mEspecialUsuario;
	CComando(QByteArray comando, bool binario, int tamanhoRespostaEsperada, bool especialUsuario): mComando(comando), mBinario(binario), mTamanhoRespostaEsperada(tamanhoRespostaEsperada), mEspecialUsuario(especialUsuario) { }
};


// * * * * * * MACROS * * * * *

#define DEFINE_PROPERTY_VIRTUAL_DEFINICAO(Tipo, Nome) \
	public: \
		Property<IDispositivoMotor,Tipo,READ_WRITE> p_##Nome; \
	protected: \
		volatile Tipo m##Nome; \
	public: \
		virtual Tipo pegar##Nome() = 0; \
		virtual void gravar##Nome(Tipo) = 0; \
	public \
	Q_SLOTS: \
		virtual void slot##Nome##_Mudou(Tipo valor) = 0; \
	private: \
	Q_SIGNALS: \
		virtual void signal##Nome##_Mudou(Tipo valor) = 0; \

#define DEFINE_PROPERTY_VIRTUAL_IMPLEMENTACAO(Tipo, Nome) \
	public: \
		Tipo pegar##Nome()	{	return m##Nome;	} \
	void gravar##Nome(Tipo valor)	{ m##Nome = valor; } \
	public \
	Q_SLOTS: \
		void slot##Nome##_Mudou(Tipo valor) {	m##Nome = valor; }


#if defined(_DEBUG) || defined(_DETALHE_MOTOR_APPLIED) || defined(_DETALHE_MOTOR_TESTE)

#  include <Erros.h>
#  include <iostream>

#  define DEFINE_PROPERTY_LOCAL_DEFINICAO(Tipo, Nome) \
	public: \
		Property<IDispositivoMotor,Tipo,READ_WRITE> p_##Nome; \
	protected: \
		volatile Tipo m##Nome; \
	public: \
		Tipo pegar##Nome()	{	return m##Nome;	} \
	public \
	Q_SLOTS: \
		void slot##Nome##_Mudou(Tipo valor);

#else

#  define DEFINE_PROPERTY_LOCAL_DEFINICAO(Tipo, Nome) \
	public: \
		Property<IDispositivoMotor,Tipo,READ_WRITE> p_##Nome; \
	protected: \
		volatile Tipo m##Nome; \
	public: \
		Tipo pegar##Nome()	{	return m##Nome;	} \
	public \
	Q_SLOTS: \
		void slot##Nome##_Mudou(Tipo valor);

#endif



class IDispositivoMotor: public QThread {
	Q_OBJECT

public:
	enum tipo {
		TipoNulo       = 0,
		TipoCamera1    = 1,
		TipoCamera2    = 2,
		TipoCamera3    = 3,
		TipoCamera4    = 4,
		TipoCamera5    = 5,
		TipoCamera6    = 6,
		TipoCamera7    = 7,
		TipoCamera8    = 8,
		TipoCilindro   = 9,
		TipoIndefinido = 10
	};

	enum sentido {
		Esquerda,	// positiva
		Direita,	// negativa
		Frente,
		Tras
	};

	enum intensidade {
		VelocidadeBaixa,
		VelocidadeMedia,
		VelocidadeAlta
	};

	enum borda {
		Subida,
		Descida
	};

public:

	//! \brief Construtora padrão.
	//! \param dadosConfiguracao Objeto manipulador dos dados de _Configuração_.
	//! \param pai Para efeito de compatibilidade, o __QObject__ de onde deriva esta classe, será inicializado com este parâmetro.
	IDispositivoMotor(CDadosConfiguracao * dadosConfiguracao, QObject * pai = nullptr);
	~IDispositivoMotor();
	virtual void procurarPortas();
	void run();
	QString ajustaValor(qint32 numero, int decimais);
	bool abrirPorta(QString nomePorta = QString(""));
	void fecharPorta(bool obriga = false);
	void ajustarVelocidadePorta(int velocidade);
	void enviaComando(QString comando, bool prioritario = false, bool especialUsuario = false);
	void enviaComandoHex(QString comandoHex, bool usaSeparador = true, int tamanhoRespostaEsperada = 0);
	void removerComandoTransmitido(bool remove);
	virtual void restringirListaPortas() = 0;
	virtual bool inicializar() = 0;
	bool valorEstado(int ordem, QString * mensagem = nullptr);
	bool valorAlarme(int ordem, QString * mensagem = nullptr);
	bool valorEntrada(int ordem, QString * mensagem = nullptr);
	virtual bool valorEntradaFimCurso(QString * mensagem = nullptr) = 0;
	virtual bool valorEntradaPressostato(QString * mensagem = nullptr) = 0;
	bool valorSaida(int ordem, QString * mensagem = nullptr);
	bool emMovimento()							{	return mEmMovimento;			}
	QString tipoEmNome(IDispositivoMotor::tipo tipo);
	QStringList portasDisponiveis()				{	return mPortasDisponiveis;				}
	QStringList portasComDispositivos()			{	return mRestringindoPortasListaPortas;	}
	QStringList modelosDisponiveis()			{	return mRestringindoPortasListaModelos;			}
	QList<IDispositivoMotor::tipo> tiposDisponiveis()	{	return mRestringindoPortasListaTipos;	}
	QString	enderecoDispositivo()				{	return mEnderecoDispositivo;	}
	bool inicializado()							{	return mInicializado;			}
	bool portaAberta();
	tipo tipoDispositivo()						{	return mTipoDispositivo;			}
	QString modeloDispositivo()					{	return mModeloProduto;				}
	QString nomeDispositivo()					{	return mNomeDispositivo;			}
	QString nomePorta();
	void ajustarTipoDispositivo(tipo tipo)		{	mTipoDispositivo = tipo;	mNomeDispositivo = tipoEmNome(tipo);	}
	qint32 posicaoAtualPassos()					{	return mPosicaoPassos;		}
	qint32 posicaoAtualEncoder()				{	return mPosicaoEncoder;		}
	qint32 posicaoAtualUm()						{	return qint32( qreal(mPosicaoEncoder) / mCoeficientePos_uMParaPosEncoder );	}
	void ajustaLimiteSuperior(qint32 valor)		{	mLimiteSuperior = valor;	}
	void ajustaLimiteInferior(qint32 valor)		{	mLimiteInferior = valor;	}
	int quantidadeAlarmes()						{	return mAlarmesEstados.size();	}
	int quantidadeEstados()						{	return mEstadosEstados.size();	}
	int quantidadeEntradas()					{	return mEntradasEstados.size();	}
	int quantidadeSaidas()						{	return mSaidasEstados.size();	}

	virtual void efetivarConfiguracao() = 0;
	virtual void tratarResposta() = 0; // QByteArray bufferEntrada, QByteArray bufferSaida);
	virtual void reiniciarFabricaDispositivo() = 0;
	virtual void configurarParaTipo(IDispositivoMotor:: tipo, QString porta) = 0;
	void configurarTipo(IDispositivoMotor::tipo tipo, QString porta = "");
	virtual void zerarAlarme() = 0;
	virtual void zerarPassos() = 0;
	virtual void ajustarPosicao(qint32 posicao_uM) = 0;
	virtual void ajustarPosicaoInicialReferencia() = 0;
	virtual void moveContinuoNegativo(qint32 limite_uM = 2147483647) = 0;	// este é o maior número para um qint32 e serve como código (chamada sem parâmetro) para se decidir se realmente usa o limite ou se é contínuo mesmo;
	virtual void moveContinuoPositivo(qint32 limite_uM = 2147483647) = 0;	// este é o maior número para um qint32 e serve como código (chamada sem parâmetro) para se decidir se realmente usa o limite ou se é contínuo mesmo;
	virtual void moveContinuoInterrompe() = 0;
	virtual void movePosicaoPassosAbsoluta(qint32 posPassos) = 0;
	virtual void movePosicaoPassosRelativa(qint32 posPassos) = 0;
	virtual void movePosicaoEncoderAbsoluta(qint32 posEncoder) = 0;
	virtual void movePosicaoEncoderRelativa(qint32 posEncoder) = 0;
	virtual void movePosicaoUmAbsoluta(qint32 posUm) = 0;
	virtual void movePosicaoUmRelativa(qint32 posUm) = 0;
	virtual void ativaSaida(int ordem, bool estado) = 0;
	virtual void travarElevador(bool estado) = 0;
	virtual void travarFreio(bool estado) = 0;
	virtual void buscarFimCurso(IDispositivoMotor::sentido direcao = IDispositivoMotor::Direita, IDispositivoMotor::borda borda = IDispositivoMotor::Subida , int distanciaDesaceleracao = 6350) = 0;
	virtual void modoTeste(bool modo, QString porta) = 0;
	virtual void liberarDispositivo(IDispositivoMotor::tipo t) = 0;
	virtual void finalizar() = 0;
#ifndef	QTSERIALTEST	// definido apenas no arquivo do projeto "QtSerialTest"
	void desconectarSinaisConfiguracao();
#endif

	virtual void iniciarTesteVaiVolta(qint32 quant_uM) = 0;
	virtual void finalizarTesteVaiVolta() = 0;

	virtual void usarVelocidadeAlta() = 0;
	virtual void usarVelocidadeBaixa() = 0;
	virtual void usarVelocidadeMedia() = 0;
	virtual void usarVelocidadeMaxima() = 0;
	virtual void usarVelocidadeMinima() = 0;
	virtual void usarVelocidade(qint32 valor) = 0;

protected:
	virtual void enviaSolicitacaoEstado() = 0;
	void enviaComandoFila();
	void erroGrave();

private:
	int								mValorTempoTimeOut;

protected:
	CDadosConfiguracao 				* mDadosConfiguracao;
	bool							mInicializado;
	bool							mSinalizadoInicializado;
	volatile bool					mRodando;
	QObject							* mPai;
	QList<CComando>					mFilaComandos;
	volatile int					mIndiceMensagem;
	QTimer							* mTimerTimeOut;
	volatile bool					mPermanecerRodando;
	volatile int					mComandosEmProcessamento;
	tipo							mTipoDispositivo;
	volatile bool					mPronto;
	volatile bool					mMovendoContinuo;

	qreal							mCoeficientePosEncoderParaPosPassos;
	qreal							mCoeficientePos_uMParaPosEncoder;
	qreal							mCoeficientePos_uMParaPosPassos;
	qreal							mCoeficientePosAnguloParaPosPassos;
	qreal							mCoeficientePosAnguloParaPosEncoder;
	volatile qint32					mLimiteSuperior;
	volatile qint32					mLimiteInferior;
	volatile bool					mLimiteAtingido;
	qint32							mVelocidade;
	intensidade						mVelocidadeIntensidade;
	QString							mNomeDispositivo;
	sentido							mSentido;

	bool							mModoTeste;
	QTimer							mModoTesteTimer;
	sentido							mModoTesteSentido;
	qint32							mModoTesteVelocidadeAntiga;
	qint32							mModoTesteAceleracaoAntiga;
	qint32							mModoTesteResolucaoAntiga;

	QMutex							mMutex;
	QMutex							mMutexEnviaComando;
	QMutex							mMutexEnviaComandoFila;
	QextSerialPort					* mPortaSerial;
	QString							mPortaNome;
	QStringList						mPortasDisponiveis;
	int								mRestringindoPortasIndice;
	QStringList						mRestringindoPortasListaPortas;
	QStringList						mRestringindoPortasListaModelos;
	QList<IDispositivoMotor::tipo>	mRestringindoPortasListaTipos;
	QString							mModeloProduto;
	char							mTerminador;
	QString							mCaracteresEspeciais;
	volatile bool					mEstadoComandoTotalmenteTransmitido;
	QByteArray						mBufferSaida;
	QByteArray						mBufferEntrada;
	QByteArray						mBufferRecebido;
	QByteArray						mBufferComando;
	QString							mBufferSaidaAntigo;
	QString							mEnderecoDispositivo;
	QString							mMensagemComandoReconhecido;
	volatile qint32					mPosicaoPassos;
	volatile qint32					mPosicaoPassosAnterior;
	volatile qint32					mPosicaoEncoder;
	volatile qint32					mPosicao_uM;

	//! \brief	Trata-se da posição imediatamente anterior à atual durante a leitura de posição em movimento.
	volatile qint32					mPosicaoEncoderAnterior;

	//! \brief	Trata-se da posição em que o dispositivo se encontrava antes de iniciar um movimento.
	volatile qint32					mPosicaoEncoderAntiga;

	volatile qint32					mPosicaoEncoderDiferenca;
	volatile quint16				mAlarmes;
	volatile quint16				mAlarmesAnteriores;
	volatile quint16				mEstados;
	volatile quint16				mEstadosAnteriores;
	volatile quint16				mSaidas;
	volatile quint16				mSaidasAnteriores;
	volatile quint16				mEntradas;
	volatile quint16				mEntradasAnteriores;
	QStringList						mAlarmesMensagens;
	QVector<bool>					mAlarmesEstados;
	QStringList						mEstadosMensagens;
	QVector<bool>					mEstadosEstados;
	QMap<tipo, QStringList>			mEntradasMensagens;
	QVector<bool>					mEntradasEstados;
	QMap<tipo, QStringList>			mSaidasMensagens;
	QVector<bool>					mSaidasEstados;
	volatile qint32					mPosicaoEncoderDesejada;
	qint32							mPosicaoEncoderDesejadaTolerancia;
	volatile qint32					mPosicaoPassosDesejada;
	volatile int					mContadorFalhaTransmissao;
	volatile int					mContadorRetentativasTransmissao;
	volatile bool					mRemoverComandoDaFila;
	volatile bool					mComandoReconhecido;
	volatile bool					mMonitorandoPosicaoEncoder;
	volatile bool					mMonitorandoPosicaoPassos;
	bool							mTorqueLigado;
	volatile bool					mEmMovimento;
	volatile bool					mEmMovimentoAnterior;
	volatile bool					mSemErroComunicacao;

	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, Aceleracao)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, CorrenteMovimento)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, CorrenteParado)

	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, DefasagemCentroLente)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, DistanciaReferenciaRelativa)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, DividendoEncoder_uM)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, DivisorEncoder_uM)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, DivisorMotorEncoder)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, DividendoMotorEncoder)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, MargemMinimaNegativa)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, MargemMinimaPositiva)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, MovimentoGrande)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, MovimentoMedio)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, MovimentoPequeno)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, PosicaoReferenciaAbsoluta)

	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, ResolucaoMotor)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, SensorAtivado)

	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, VelocidadeAlta)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, VelocidadeBaixa)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, VelocidadeMedia)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, VelocidadeMaxima)
	DEFINE_PROPERTY_LOCAL_DEFINICAO(qint32, VelocidadeMinima)

public
Q_SLOTS:
	void slotPortaTimeOut();
	void slotLidoCompletamente();
	void slotLidoParcialmente();
	virtual void slotMudancaTipoDetectado(IDispositivoMotor::tipo tipoAntigo, IDispositivoMotor::tipo tipoNovo, QString porta) = 0;
	virtual void slotRestringirListaPortasContinua() = 0;
	virtual void slotReabrirPorta() = 0;
	virtual void slotReceberPosicaoDestinoAbsoluta(IDispositivoMotor::tipo tipoDispositivo, qint32 posicao) = 0;
	virtual void slotIniciarMovimentoContinuo(IDispositivoMotor::tipo tipoDispositivo, IDispositivoMotor::sentido s, IDispositivoMotor::intensidade v, qint32 limiteSuperior, qint32 limiteInferior) = 0;
	virtual void slotPararMovimentoContinuo(IDispositivoMotor::tipo tipoDispositivo) = 0;
	virtual void slotModoTesteTimerTimeout() = 0;
	virtual void slotVelocidadeIntensidade(IDispositivoMotor::tipo tipoDispositivo, IDispositivoMotor::intensidade vel) = 0;

public:
Q_SIGNALS:
#if defined(_DEBUG) || defined(_DETALHE_MOTOR_APPLIED) || defined(_DETALHE_MOTOR_TESTE)
	void Log(int,QString,QString);
#endif
	void sinalDispositivosEncontrados();
	void sinalLidoCompletamente();
	void sinalLidoParcialmente();
	void sinalMudancaTipoDetectado(IDispositivoMotor::tipo tipoAntigo, IDispositivoMotor::tipo tipoNovo, QString porta);
	void sinalAlarme(IDispositivoMotor::tipo tipoMotor, quint16 alarme);
	void sinalEstado(IDispositivoMotor::tipo tipoMotor, quint16 estado);
	void sinalSaida(IDispositivoMotor::tipo tipoMotor, quint16 saidas);
	void sinalEntrada(IDispositivoMotor::tipo tipoMotor, quint16 entradas);
	void sinalPosicaoPassosAtual(IDispositivoMotor::tipo tipoMotor, qint32 posicaoPassos);
	void sinalPosicaoEncoderAtual(IDispositivoMotor::tipo tipoMotor, qint32 posicaoEncoder);
	void sinalEstadoComunicacao(IDispositivoMotor::tipo tipoMotor, bool estado);
	void sinalDispositivoEmMovimento(IDispositivoMotor::tipo tipoMotor, bool estado);
	void sinalPosicao_uMAtual(IDispositivoMotor::tipo tipoMotor, qint32 posicao);
	void sinalInicializado(IDispositivoMotor::tipo tipoMotor);
	void sinalRespostaAComandoUsuario(IDispositivoMotor::tipo tipoMotor, QString resposta);

	friend class CDispositivoMaquina;
	friend class CDadosConfiguracao;
};

#endif // IMOTOR_H
