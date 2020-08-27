// "$Date: 2018-03-23 13:11:01 -0300 (sex, 23 mar 2018) $"
// "$Author: fares $"
// "$Revision: 4195 $"

#include "QtSerialTest.h"

#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
#include "LogNetSender.h"
#include "Erros.h"
#endif

#if defined(_DEBUG)
#  include <QDebug>
#endif

#include "CDispositivoMotorApplied.h"
#include <QTableWidgetItem>
#include <QFontDatabase>
#include <QMessageBox>
#include "../QtCliche/NomesDispositivos.h"


QtSerialTest::QtSerialTest(QWidget *pai) : QWidget(pai) {
	setupUi(this);

	// setAttribute(Qt::WA_DeleteOnClose, true);

	QFontDatabase::addApplicationFont(":/fontes/Digir___.ttf");
	QFontDatabase::addApplicationFont(":/fontes/Digirc__.ttf");
	QFontDatabase::addApplicationFont(":/fontes/Digircu_.ttf");
	QFontDatabase::addApplicationFont(":/fontes/Digire__.ttf");
	QFontDatabase::addApplicationFont(":/fontes/Digireu_.ttf");
	QFontDatabase::addApplicationFont(":/fontes/Digirtu_.ttf");
	QFontDatabase::addApplicationFont(":/fontes/LEDBDREV.TTF");
	QFontDatabase::addApplicationFont(":/fontes/LEDBOARD.TTF");

	lbVersao->setText(QString::fromUtf8("Versão: %1").arg(REV_CODE));

#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
	mLogSender = new LogNetSender(NULL, 8);
	mLogSender->moveToThread(&mThreadLog);
	mThreadLog.start();
	connect(this, SIGNAL(Log(int,QString,QString,QString)), mLogSender, SLOT(Log(int,QString,QString,QString)));
#endif
	mDispositivo = new CDispositivoMotorApplied(NULL, this);
#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
	connect(mDispositivo, SIGNAL(Log(int,QString,QString,QString)), this, SIGNAL(Log(int,QString,QString,QString)));
#endif
	mDispositivo->procurarPortas();
	if ( ! mDispositivo->portasDisponiveis().isEmpty()) {

		mDispositivo->restringirListaPortas();
		connect(mDispositivo, SIGNAL(sinalDispositivosEncontrados()), this, SLOT(slotDispositivosEncontrados()));
		connect(mDispositivo, SIGNAL(sinalPosicao_uMAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicao_uMAtual(IDispositivoMotor::tipo, qint32)));
		connect(mDispositivo, SIGNAL(sinalPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)));
		connect(mDispositivo, SIGNAL(sinalAlarme(IDispositivoMotor::tipo, quint16)), this, SLOT(slotAlarme(IDispositivoMotor::tipo, quint16)));
		connect(mDispositivo, SIGNAL(sinalEstado(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEstado(IDispositivoMotor::tipo, quint16)));
		connect(mDispositivo, SIGNAL(sinalDispositivoEmMovimento(IDispositivoMotor::tipo,bool)), this, SLOT(slotDispositivoEmMovimento(IDispositivoMotor::tipo,bool)));
		connect(mDispositivo, SIGNAL(sinalEntrada(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEntrada(IDispositivoMotor::tipo, quint16)));
		connect(mDispositivo, SIGNAL(sinalSaida(IDispositivoMotor::tipo, quint16)), this, SLOT(slotSaida(IDispositivoMotor::tipo, quint16)));
		connect(mDispositivo, SIGNAL(sinalEstadoComunicacao(IDispositivoMotor::tipo,bool)), this, SLOT(slotEstadoComunicacao(IDispositivoMotor::tipo,bool)));
		connect(mDispositivo, SIGNAL(sinalInicializado(IDispositivoMotor::tipo)), this, SLOT(slotInicializado(IDispositivoMotor::tipo)));
		connect(mDispositivo, SIGNAL(sinalRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)), this, SLOT(slotRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)));

		pbAbrirPorta->setEnabled(false);
		gbEstados->setEnabled(false);
		gbAlarmes->setEnabled(false);
		gbEncoder->setEnabled(false);
		gbPassos->setEnabled(false);
		gbProporcao->setEnabled(false);
		gbVelocidade->setEnabled(false);
		gbJog->setEnabled(false);
		gbEntradas->setEnabled(false);
		gbSaidas->setEnabled(false);
		gbComandoUsuario->setEnabled(false);
		leVaiVoltaQtd->setEnabled(false);
		pbTesteVaiVolta->setEnabled(false);

		QString aux1;
		bool aux2;
		QTableWidgetItem * item;

		twAlarmes->setRowCount(0);
		twAlarmes->setRowCount(mDispositivo->quantidadeAlarmes());
		for (int i = 0; i < mDispositivo->quantidadeAlarmes(); i++) {
			aux2 = mDispositivo->valorAlarme(i, &aux1);
			item = new QTableWidgetItem(aux1);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			twAlarmes->setItem(i, 0, item);
			item = new QTableWidgetItem();
			if (aux2) {
				item->setIcon(QIcon(":/icones/1"));
			} else {
				item->setIcon(QIcon(":/icones/0"));
			}
			twAlarmes->setItem(i, 1, item);
		}

		twEstados->setRowCount(0);
		twEstados->setRowCount(mDispositivo->quantidadeEstados());
		for (int i = 0; i < mDispositivo->quantidadeEstados(); i++) {
			aux2 = mDispositivo->valorEstado(i, &aux1);
			item = new QTableWidgetItem(aux1);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			twEstados->setItem(i, 0, item);
			item = new QTableWidgetItem();
			if (aux2) {
				item->setIcon(QIcon(":/icones/1"));
			} else {
				item->setIcon(QIcon(":/icones/0"));
			}
			twEstados->setItem(i, 1, item);
		}

		twEntradas->setRowCount(0);
		twEntradas->setRowCount(mDispositivo->quantidadeEntradas());
		for (int i = 0; i < mDispositivo->quantidadeEntradas(); i++) {
			aux2 = mDispositivo->valorEntrada(i, &aux1);
			item = new QTableWidgetItem(aux1);
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			twEntradas->setItem(i, 1, item);
			item = new QTableWidgetItem();
			if (aux2) {
				item->setIcon(QIcon(":/icones/1"));
			} else {
				item->setIcon(QIcon(":/icones/0"));
			}
			twEntradas->setItem(i, 0, item);
		}

		twSaidas->setRowCount(0);
		twSaidas->setRowCount(mDispositivo->quantidadeSaidas());
		for (int i = 0; i < mDispositivo->quantidadeSaidas(); i++) {
			aux2 = mDispositivo->valorSaida(i, &aux1);
			item = new QTableWidgetItem(aux1);
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			twSaidas->setItem(i, 2, item);
			item = new QTableWidgetItem();
			if (aux2) {
				item->setIcon(QIcon(":/icones/1"));
			} else {
				item->setIcon(QIcon(":/icones/0"));
			}
			twSaidas->setItem(i, 0, item);
			item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsUserCheckable);
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			if (aux2) {
				item->setCheckState(Qt::Checked);
			} else {
				item->setCheckState(Qt::Unchecked);
			}
			twSaidas->setItem(i, 1, item);
		}

		twAlarmes->resizeColumnsToContents();
		twEstados->resizeColumnsToContents();
		twEntradas->resizeColumnsToContents();
		twSaidas->resizeColumnsToContents();

		int largura = 200;
		twAlarmes->horizontalHeader()->resizeSection(0, largura - 20);
		twEstados->horizontalHeader()->resizeSection(0, largura - 20);
		// twAlarmes->horizontalHeader()->resizeSection(1, 18);
		// twEstados->horizontalHeader()->resizeSection(1, 18);
		twAlarmes->horizontalHeader()->stretchLastSection();
		twEstados->horizontalHeader()->stretchLastSection();

		twEntradas->horizontalHeader()->resizeSection(0, 20);
		twEntradas->horizontalHeader()->stretchLastSection();

		twSaidas->horizontalHeader()->resizeSection(0, 20);
		twSaidas->horizontalHeader()->resizeSection(1, 20);
		twSaidas->horizontalHeader()->stretchLastSection();

		mEditandoMoverEncoder = false;
		mEditandoMoverPassos = false;

		QTableWidgetItem teste; teste.setCheckState(Qt::Unchecked);

		mPosEncoder = 0;
		lbComunicacao_1->setPixmap(QPixmap(":/icones/0"));
		lbComunicacao_2->setText("procurando dispositivos...");

		mPortaAberta = false;
		mPortaAcabouAbrirVerificaSaidas = false;
		mMensagensAtualizadas = false;
		mEmMovimento = false;
		mMovendoEncoder = false;

		mPosPassosDesejada = 0;
		mPosEncoderDesejada = 0;
	} else {
		delete mDispositivo;
		mDispositivo = NULL;
	}
	mEmTesteVaiVolta = false;

	mTimerFaltaResposta.setSingleShot(true);
	mTimerFaltaResposta.setInterval(5000);
	connect (&mTimerFaltaResposta, SIGNAL(timeout()), this, SLOT(slotSemRespostaHabilitaInterface()));
#if defined(_DEBUG)
	qDebug() << "QtSerialTest: comecei.";
#endif
}

QtSerialTest::~QtSerialTest() {
	if (mTimerFaltaResposta.isActive()) {
		mTimerFaltaResposta.stop();
	}
	disconnect (&mTimerFaltaResposta, SIGNAL(timeout()), this, SLOT(slotSemRespostaHabilitaInterface()));
	if ( mDispositivo != NULL ) {
		on_pbAbrirPorta_clicked(false);	// fecha a porta
		mThreadLog.quit();
		mThreadLog.wait();

		foreach (mDispositivo, mDispositivos.values()) {
			disconnect(mDispositivo, SIGNAL(sinalPosicao_uMAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicao_uMAtual(IDispositivoMotor::tipo, qint32)));
			disconnect(mDispositivo, SIGNAL(sinalPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)));
			disconnect(mDispositivo, SIGNAL(sinalAlarme(IDispositivoMotor::tipo, quint16)), this, SLOT(slotAlarme(IDispositivoMotor::tipo, quint16)));
			disconnect(mDispositivo, SIGNAL(sinalEstado(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEstado(IDispositivoMotor::tipo, quint16)));
			disconnect(mDispositivo, SIGNAL(sinalDispositivoEmMovimento(IDispositivoMotor::tipo,bool)), this, SLOT(slotDispositivoEmMovimento(IDispositivoMotor::tipo,bool)));
			disconnect(mDispositivo, SIGNAL(sinalEntrada(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEntrada(IDispositivoMotor::tipo, quint16)));
			disconnect(mDispositivo, SIGNAL(sinalSaida(IDispositivoMotor::tipo, quint16)), this, SLOT(slotSaida(IDispositivoMotor::tipo, quint16)));
			disconnect(mDispositivo, SIGNAL(sinalEstadoComunicacao(IDispositivoMotor::tipo,bool)), this, SLOT(slotEstadoComunicacao(IDispositivoMotor::tipo,bool)));
			disconnect(mDispositivo, SIGNAL(sinalInicializado(IDispositivoMotor::tipo)), this, SLOT(slotInicializado(IDispositivoMotor::tipo)));
			disconnect(mDispositivo, SIGNAL(sinalRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)), this, SLOT(slotRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)));
#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
			disconnect(mDispositivo, SIGNAL(Log(int,QString,QString,QString)), this, SIGNAL(Log(int,QString,QString,QString)));
#endif
			delete mDispositivo;
		}
		mDispositivos.clear();
		mDispositivo = NULL;

#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
		disconnect(this, SIGNAL(Log(int,QString,QString,QString)), mLogSender, SLOT(Log(int,QString,QString,QString)));
		mLogSender->finish();
		delete mLogSender;
#endif
	}
}

void QtSerialTest::changeEvent(QEvent *e) {
	QWidget::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			retranslateUi(this);
			break;
		default:
			break;
	}
}

void QtSerialTest::slotDispositivosEncontrados() {
	lbComunicacao_1->setPixmap(QPixmap(":/icones/0"));
	if ( mDispositivo->portasDisponiveis().size() > 0 ) {
		if ( ( mDispositivo->portasComDispositivos().size() >= 2 ) && ( mDispositivo->portasComDispositivos().size() <= 3 ) ) {
			// pode ter dois dispositivos no caso de uma montadora com movimentação motorizada nas câmeras, mas manual no cilindro;
			int desconhecidos = mDispositivo->tiposDisponiveis().count(IDispositivoMotor::TipoIndefinido);
			if (desconhecidos > 0) {
				QList<IDispositivoMotor::tipo> existentes = mDispositivo->tiposDisponiveis();
				QList<IDispositivoMotor::tipo> possibilidades ;
				possibilidades << IDispositivoMotor::TipoCamera1 << IDispositivoMotor::TipoCamera2 ;
				if ( mDispositivo->portasComDispositivos().size() == 3 ) {
					possibilidades << IDispositivoMotor::TipoCilindro;
				}
				foreach (IDispositivoMotor::tipo existente , existentes) {
					if (possibilidades.contains(existente)) {
						possibilidades.removeOne(existente);
					}
				}
				while (desconhecidos > 1) {
					IDispositivoMotor::tipo tipoTeste;
					QString porta;
					bool resposta = false;
					porta = mDispositivo->portasComDispositivos().at( mDispositivo->tiposDisponiveis().indexOf(IDispositivoMotor::TipoIndefinido) );
					mDispositivo->modoTeste(true, porta);
					while ( ! resposta) {
						foreach (tipoTeste, possibilidades) {
							QString msg = QString::fromUtf8("Este é o motor\n%1 ?").arg(mDispositivo->tipoEmNome(tipoTeste));
							QMessageBox * mb = new QMessageBox(QMessageBox::Question, "Identifique o motor", msg, QMessageBox::Yes | QMessageBox::No, this);
							resposta = ( mb->exec() == QMessageBox::Yes );
							if (resposta) {
								break;
							}
						}
					}
					possibilidades.removeOne(tipoTeste);
					mDispositivo->modoTeste(false, porta);
					mDispositivo->configurarParaTipo(tipoTeste, porta);
					mDispositivo->fecharPorta();
					desconhecidos = mDispositivo->tiposDisponiveis().count(IDispositivoMotor::TipoIndefinido);
				}
				if (desconhecidos == 1) {
					QList<IDispositivoMotor::tipo> existentes = mDispositivo->tiposDisponiveis();
					QList<IDispositivoMotor::tipo> possibilidades ;
					possibilidades << IDispositivoMotor::TipoCamera1 << IDispositivoMotor::TipoCamera2 ;
					if ( mDispositivo->portasComDispositivos().size() == 3 ) {
						possibilidades << IDispositivoMotor::TipoCilindro;
					}
					foreach (IDispositivoMotor::tipo existente , existentes) {
						if (possibilidades.contains(existente)) {
							possibilidades.removeOne(existente);
						}
					}
					QString porta = mDispositivo->portasComDispositivos().at( mDispositivo->tiposDisponiveis().indexOf(IDispositivoMotor::TipoIndefinido) );
					mDispositivo->configurarParaTipo(possibilidades.first(), porta);
					mDispositivo->fecharPorta();
				}
			}
			/*
			for (int i=0; i < mDispositivo->tiposDisponiveis().size(); i++) {
				cbPortas->addItem(mDispositivo->tipoEmNome(mDispositivo->tiposDisponiveis().at(i)));
			}
			*/
			for (int i = 0; i < mDispositivo->quantidadeEstados() ; i++) {
				twEstados->item(i,1)->setIcon(QIcon(":/icones/0"));
			}
			for (int i = 0 ; i < mDispositivo->quantidadeAlarmes() ; i++) {
				twAlarmes->item(i,1)->setIcon(QIcon(":/icones/0"));
			}
			for (int i = 0; i < mDispositivo->quantidadeSaidas() ; i++) {
				twSaidas->item(i,0)->setIcon(QIcon(":/icones/0"));
				twSaidas->item(i,1)->setCheckState(Qt::Unchecked);
			}
			for (int i = 0 ; i < mDispositivo->quantidadeEntradas() ; i++) {
				twEntradas->item(i,0)->setIcon(QIcon(":/icones/0"));
			}
		} // else {
			cbPortas->addItems(mDispositivo->portasDisponiveis());
		//}
		cbPortas->setCurrentIndex(0);

		foreach ( IDispositivoMotor::tipo tipo, mDispositivo->tiposDisponiveis() ) {
			int index = mDispositivo->tiposDisponiveis().indexOf(tipo);
			IDispositivoMotor * dispositivo = new CDispositivoMotorApplied(NULL, this);
			dispositivo->configurarParaTipo(tipo, mDispositivo->portasComDispositivos().at(index));
			mDispositivos[mDispositivo->portasComDispositivos().at(index)] = dispositivo;
#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
			connect(dispositivo, SIGNAL(Log(int,QString,QString,QString)), this, SIGNAL(Log(int,QString,QString,QString)));
#endif
			connect(dispositivo, SIGNAL(sinalPosicao_uMAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicao_uMAtual(IDispositivoMotor::tipo, qint32)));
			connect(dispositivo, SIGNAL(sinalPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)));
			connect(dispositivo, SIGNAL(sinalAlarme(IDispositivoMotor::tipo, quint16)), this, SLOT(slotAlarme(IDispositivoMotor::tipo, quint16)));
			connect(dispositivo, SIGNAL(sinalEstado(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEstado(IDispositivoMotor::tipo, quint16)));
			connect(dispositivo, SIGNAL(sinalDispositivoEmMovimento(IDispositivoMotor::tipo,bool)), this, SLOT(slotDispositivoEmMovimento(IDispositivoMotor::tipo,bool)));
			connect(dispositivo, SIGNAL(sinalEntrada(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEntrada(IDispositivoMotor::tipo, quint16)));
			connect(dispositivo, SIGNAL(sinalSaida(IDispositivoMotor::tipo, quint16)), this, SLOT(slotSaida(IDispositivoMotor::tipo, quint16)));
			connect(dispositivo, SIGNAL(sinalEstadoComunicacao(IDispositivoMotor::tipo,bool)), this, SLOT(slotEstadoComunicacao(IDispositivoMotor::tipo,bool)));
			connect(dispositivo, SIGNAL(sinalInicializado(IDispositivoMotor::tipo)), this, SLOT(slotInicializado(IDispositivoMotor::tipo)));
			connect(dispositivo, SIGNAL(sinalRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)), this, SLOT(slotRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)));
		}

		if (-1 != mDispositivo->portasComDispositivos().indexOf(cbPortas->currentText()) ) {
			cbPortas->setToolTip( mDispositivo->tipoEmNome( mDispositivo->tiposDisponiveis().at( mDispositivo->portasComDispositivos().indexOf( cbPortas->currentText() ) ) ) );
		}
		pbAbrirPorta->setEnabled(true);
		lbComunicacao_2->setText(QString::fromUtf8("Aguardando usuário..."));
	} else {
		lbComunicacao_2->setText("NENHUM DISPOSITIVO ENCONTRADO.");
		QMessageBox * mb = new QMessageBox(QMessageBox::Question, QString::fromUtf8("Detecção nula"), "Nenhum dispositivo\nfoi encontrado.\n\nEncerrar programa?", QMessageBox::Yes | QMessageBox::No, this);
		if ( mb->exec() == QMessageBox::Yes ) {
			close();
		}
	}
#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
	disconnect(mDispositivo, SIGNAL(Log(int,QString,QString,QString)), this, SIGNAL(Log(int,QString,QString,QString)));
#endif
	disconnect(mDispositivo, SIGNAL(sinalPosicao_uMAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicao_uMAtual(IDispositivoMotor::tipo, qint32)));
	disconnect(mDispositivo, SIGNAL(sinalPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)), this, SLOT(slotPosicaoPassosAtual(IDispositivoMotor::tipo, qint32)));
	disconnect(mDispositivo, SIGNAL(sinalAlarme(IDispositivoMotor::tipo, quint16)), this, SLOT(slotAlarme(IDispositivoMotor::tipo, quint16)));
	disconnect(mDispositivo, SIGNAL(sinalEstado(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEstado(IDispositivoMotor::tipo, quint16)));
	disconnect(mDispositivo, SIGNAL(sinalDispositivoEmMovimento(IDispositivoMotor::tipo,bool)), this, SLOT(slotDispositivoEmMovimento(IDispositivoMotor::tipo,bool)));
	disconnect(mDispositivo, SIGNAL(sinalEntrada(IDispositivoMotor::tipo, quint16)), this, SLOT(slotEntrada(IDispositivoMotor::tipo, quint16)));
	disconnect(mDispositivo, SIGNAL(sinalSaida(IDispositivoMotor::tipo, quint16)), this, SLOT(slotSaida(IDispositivoMotor::tipo, quint16)));
	disconnect(mDispositivo, SIGNAL(sinalEstadoComunicacao(IDispositivoMotor::tipo,bool)), this, SLOT(slotEstadoComunicacao(IDispositivoMotor::tipo,bool)));
	disconnect(mDispositivo, SIGNAL(sinalInicializado(IDispositivoMotor::tipo)), this, SLOT(slotInicializado(IDispositivoMotor::tipo)));
	disconnect(mDispositivo, SIGNAL(sinalRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)), this, SLOT(slotRespostaAComandoUsuario(IDispositivoMotor::tipo, QString)));
	delete mDispositivo;
	mDispositivo = NULL;
}

void QtSerialTest::on_cbPortas_currentIndexChanged(int item) {
	Q_UNUSED(item)
	if (-1 != mDispositivos.keys().indexOf(cbPortas->currentText()) ) {
		// cbPortas->setToolTip( mDispositivo->tipoEmNome( mDispositivo->tiposDisponiveis().at( mDispositivo->portasComDispositivos().indexOf( cbPortas->currentText() ) ) ) );
		cbPortas->setToolTip( mDispositivos[cbPortas->currentText()]->nomeDispositivo() );
	}
}

void QtSerialTest::on_pbAbrirPorta_clicked(bool estado) {
	pbAbrirPorta->setEnabled(false);
	mEmTesteVaiVolta = false;
	if (estado) {	// quero abrir a porta
		mMensagensAtualizadas = false;
		if ( (0 < mDispositivos.size() ) && ( -1 != mDispositivos.keys().indexOf(cbPortas->currentText()) ) ) {
			mDispositivo = mDispositivos[cbPortas->currentText()];	// já existe e está configurado;
		} else {
			mDispositivo = new CDispositivoMotorApplied(NULL, this);	// não foi detectado, mas vamos criar um dispositivo e tentar atuar sobre ele;
			mDispositivo->configurarParaTipo(IDispositivoMotor::TipoIndefinido, cbPortas->currentText());
		}
		mTipoDispositivoEmUso = mDispositivo->tipoDispositivo();
		if ( mDispositivo->abrirPorta(cbPortas->currentText()) ) {
			cbPortas->setEnabled(false);
			rbVelocidadeMedia->setChecked(true);
			pbAbrirPorta->setText("Fechar");
			gbEstados->setEnabled(true);
			gbAlarmes->setEnabled(true);
			gbEncoder->setEnabled(true);
			gbPassos->setEnabled(true);
			gbProporcao->setEnabled(true);
			gbVelocidade->setEnabled(true);
			gbJog->setEnabled(true);
			gbEntradas->setEnabled(true);
			gbSaidas->setEnabled(true);
			gbComandoUsuario->setEnabled(true);
			lbComunicacao_1->setPixmap(QPixmap(":/icones/0"));
			lbComunicacao_2->setText("");
			lePosPassos->setText(QString::number(mDispositivo->posicaoAtualPassos()));
			lePosEncoder->setText(QString::number(mDispositivo->posicaoAtualEncoder()));
			lePosPassos->setReadOnly(false);
			lePosEncoder->setReadOnly(false);
			pbMoverEncoder->setEnabled(true);
			mEditandoMoverEncoder = false;
			pbZeraEncoder->setEnabled(true);
			pbMoverPassos->setEnabled(true);
			mEditandoMoverPassos = false;
			pbZeraPassos->setEnabled(true);
			pbFimCursoNegativo->setEnabled(true);
			pbFimCursoPositivo->setEnabled(true);
			pbNegativo->setEnabled(true);
			pbPositivo->setEnabled(true);

			mDispositivo->inicializar();
			if (mDispositivo->tipoDispositivo() != IDispositivoMotor::TipoCilindro) {
				mDispositivo->p_Aceleracao = 352000;
				// mDispositivo->p_DefasagemCentroLente = 32000;
				// mDispositivo->p_DistanciaReferenciaRelativa = 500000;
				mDispositivo->p_DividendoEncoder_uM = 1;
				mDispositivo->p_DivisorEncoder_uM = 5;
				mDispositivo->p_DividendoMotorEncoder = 999952;
				mDispositivo->p_DivisorMotorEncoder = 43980186;	//	dividindo-se 43980186 por 999952 se obtem muito aproximadamente a PI * 14 (diâmetro primitivo)
				mDispositivo->p_CorrenteMovimento = 200;
				mDispositivo->p_CorrenteParado = 180;
				// mDispositivo->p_MargemMinimaNegativa = 39000;
				// mDispositivo->p_MargemMinimaPositiva = 39000;
				mDispositivo->p_MovimentoGrande = 1000;
				mDispositivo->p_MovimentoMedio = 100;
				mDispositivo->p_MovimentoPequeno = 10;
				// mDispositivo->p_VelocidadeMaxima = 176000;
				mDispositivo->p_VelocidadeAlta = 264000;
				mDispositivo->p_VelocidadeMedia = 132000;
				mDispositivo->p_VelocidadeBaixa = 5500;
				// mDispositivo->p_VelocidadeMinima = 880;
				/*
				if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCamera1) {
					mDispositivo->p_PosicaoReferenciaAbsoluta = 0;			// 283.872 pulsos para a câmera 2, na travessa de desenvolvimento (ou 1.419.360 um).
				} else {
					mDispositivo->p_PosicaoReferenciaAbsoluta = 1419360;	// 283.872 pulsos para a câmera 2, na travessa de desenvolvimento (ou 1.419.360 um).
				}
				*/
			} else {
				mDispositivo->p_Aceleracao = 2000;
				mDispositivo->p_CorrenteMovimento = 200;
				mDispositivo->p_CorrenteParado = 180;
				// mDispositivo->p_DefasagemCentroLente = 32000;
				// mDispositivo->p_DistanciaReferenciaRelativa = 500000;
				mDispositivo->p_DividendoEncoder_uM = 1;
				mDispositivo->p_DivisorEncoder_uM = 1;
				mDispositivo->p_DividendoMotorEncoder = 960;
				mDispositivo->p_DivisorMotorEncoder = 16;
				// mDispositivo->p_MargemMinimaNegativa = 39000;
				// mDispositivo->p_MargemMinimaPositiva = 39000;
				mDispositivo->p_MovimentoGrande = 100;
				mDispositivo->p_MovimentoMedio = 10;
				mDispositivo->p_MovimentoPequeno = 1;
				// mDispositivo->p_VelocidadeMaxima = 20000;
				mDispositivo->p_VelocidadeAlta = 10000;
				mDispositivo->p_VelocidadeMedia = 5000;
				mDispositivo->p_VelocidadeBaixa = 150;
				// mDispositivo->p_VelocidadeMinima = 10;
			}
			mDispositivo->p_ResolucaoMotor = 50800;

			mDispositivo->usarVelocidadeMedia();
			mDispositivo->efetivarConfiguracao();
			rbVelocidadeMedia->setChecked(true);
			mPortaAberta = true;

			for (int i = 0; i < mDispositivo->quantidadeEstados() ; i++) {
				twEstados->item(i,1)->setIcon(QIcon(":/icones/0"));
			}
			for (int i = 0 ; i < mDispositivo->quantidadeAlarmes() ; i++) {
				twAlarmes->item(i,1)->setIcon(QIcon(":/icones/0"));
			}
			QString aux1;
			bool aux2;
			for (int i = 0 ; i < mDispositivo->quantidadeEntradas() ; i++) {
				aux2 = mDispositivo->valorEntrada(i, &aux1);
				if (aux2) {
					twEntradas->item(i,0)->setIcon(QIcon(":/icones/1"));
				} else {
					twEntradas->item(i,0)->setIcon(QIcon(":/icones/0"));
				}
				twEntradas->item(i,1)->setText(aux1);
			}
			for (int i = 0; i < mDispositivo->quantidadeSaidas() ; i++) {
				aux2 = mDispositivo->valorSaida(i, &aux1);
				if (aux2) {
					twSaidas->item(i,0)->setIcon(QIcon(":/icones/1"));
					twSaidas->item(i,1)->setCheckState(Qt::Checked);
				} else {
					twSaidas->item(i,0)->setIcon(QIcon(":/icones/0"));
					twSaidas->item(i,1)->setCheckState(Qt::Unchecked);
				}
				twSaidas->item(i,1)->setText(aux1);
			}
		}
		leVaiVoltaQtd->setEnabled(true);
		pbTesteVaiVolta->setEnabled(true);
	} else {	// quero fechar a porta
		mDispositivo->fecharPorta(true);
		mPortaAberta = false;
		mPortaAcabouAbrirVerificaSaidas = false;
		cbPortas->setEnabled(true);
		mDispositivo->fecharPorta();
		gbEstados->setEnabled(false);
		gbAlarmes->setEnabled(false);
		gbEncoder->setEnabled(false);
		gbPassos->setEnabled(false);
		gbEntradas->setEnabled(false);
		gbSaidas->setEnabled(false);
		gbComandoUsuario->setEnabled(false);
		gbProporcao->setEnabled(false);
		gbVelocidade->setEnabled(false);
		gbJog->setEnabled(false);
		pbAbrirPorta->setText("&Abrir");
		lbComunicacao_1->setPixmap(QPixmap(":/icones/0"));
		lbComunicacao_2->setText("");

		for (int i = 0; i < mDispositivo->quantidadeEstados() ; i++) {
			twEstados->item(i,1)->setIcon(QIcon(":/icones/0"));
		}
		for (int i = 0 ; i < mDispositivo->quantidadeAlarmes() ; i++) {
			twAlarmes->item(i,1)->setIcon(QIcon(":/icones/0"));
		}

		foreach(IDispositivoMotor * dispositivo, mDispositivos.values()) {
			if (dispositivo == mDispositivo) {
				mDispositivo = NULL;	// se tiver algum igual, nada foi criado, apenas copiado;
			}
		}
		if (mDispositivo != NULL) {
			delete mDispositivo;	// deve ter sido criado ao se abrir uma porta com dispositivo desconhecido ou não detectado;
		}

		// um novo dispositivo é usado apenas para limpar a interface das características individuais de cada tipo de dispositivo, usando um "TipoIndefinido";
		mDispositivo = new CDispositivoMotorApplied(NULL, this);
		mDispositivo->ajustarTipoDispositivo(IDispositivoMotor::TipoIndefinido);

		QString aux1;
		for (int i = 0 ; i < mDispositivo->quantidadeEntradas() ; i++) {
			mDispositivo->valorEntrada(i, &aux1);
			twEntradas->item(i,0)->setIcon(QIcon(":/icones/0"));
			twEntradas->item(i,1)->setText(aux1);
		}
		for (int i = 0; i < mDispositivo->quantidadeSaidas() ; i++) {
			mDispositivo->valorSaida(i, &aux1);
			twSaidas->item(i,0)->setIcon(QIcon(":/icones/0"));
			twSaidas->item(i,1)->setCheckState(Qt::Unchecked);
			twSaidas->item(i,2)->setText(aux1);
		}
		delete mDispositivo;
		mDispositivo = NULL;

		mMensagensAtualizadas = false;
		leVaiVoltaQtd->setEnabled(false);
		pbTesteVaiVolta->setEnabled(false);
	}
	pbAbrirPorta->setEnabled(true);
}

void QtSerialTest::on_pbEnviar_clicked() {
	mDispositivo->enviaComando(leComando->text().append('\r'), false, true);
	leComando->setFocus();
	leComando->selectAll();
}

void QtSerialTest::on_pbMoverEncoder_clicked() {
	lePosPassos->setReadOnly(true);
	lePosEncoder->setReadOnly(true);
	mPosEncoderDesejada = lePosEncoder->text().toLong();
	if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCilindro) {
		mPosEncoderDesejada %= (qint32)360000;
		if (mPosEncoder != mPosEncoderDesejada) {		// verifica se precisa mesmo movimentar o cilindro
			if (abs(mPosEncoderDesejada - mPosEncoder) > (qint32)180000) {
				if (mPosEncoderDesejada > mPosEncoder) {
					mPosEncoderDesejada = mPosEncoderDesejada - (qint32)360000;
				} else {
					mPosEncoderDesejada = mPosEncoderDesejada + (qint32)360000;
				}
			}
		}
	}
	if (rbVelocidadeAlta->isChecked()) {
		mDispositivo->usarVelocidadeAlta();
	}
	if (rbVelocidadeMedia->isChecked()) {
		mDispositivo->usarVelocidadeMedia();
	}
	if (rbVelocidadeBaixa->isChecked()) {
		mDispositivo->usarVelocidadeBaixa();
	}
	mDispositivo->movePosicaoUmAbsoluta(mPosEncoderDesejada);
	mMovendoEncoder = true;
	mEditandoMoverEncoder = false;
	habilitarInterface(false);
}

void QtSerialTest::on_pbMoverPassos_clicked() {
	lePosPassos->setReadOnly(true);
	lePosEncoder->setReadOnly(true);
	mPosPassosDesejada = lePosPassos->text().toLong();
	mDispositivo->movePosicaoPassosAbsoluta(mPosPassosDesejada);
	mEditandoMoverPassos = false;
	habilitarInterface(false);
}

void QtSerialTest::on_pbZeraEncoder_clicked() {
	mDispositivo->ajustarPosicao(0);
}

void QtSerialTest::on_pbZeraPassos_clicked() {
	mDispositivo->zerarPassos();
}

void QtSerialTest::on_pbLimpaAlarme_clicked() {
	mDispositivo->zerarAlarme();
}

void QtSerialTest::on_pbPositivo_pressed() {
	mDispositivo->moveContinuoPositivo();
	habilitarInterface(false, pbPositivo);
}

void QtSerialTest::on_pbPositivo_released() {
	mDispositivo->moveContinuoInterrompe();
	mTimerFaltaResposta.start();
}

void QtSerialTest::on_pbNegativo_pressed() {
	mDispositivo->moveContinuoNegativo();
	habilitarInterface(false, pbNegativo);
}

void QtSerialTest::on_pbNegativo_released() {
	mDispositivo->moveContinuoInterrompe();
	mTimerFaltaResposta.start();
}

void QtSerialTest::on_pbFimCursoPositivo_clicked() {
	lbEncoderEmPosicao->setPixmap(QPixmap(":/icones/0"));
	lbPassosEmPosicao->setPixmap(QPixmap(":/icones/0"));
	mDispositivo->buscarFimCurso(IDispositivoMotor::Esquerda, IDispositivoMotor::Descida);	//	assume o padrão de 6350 na distância de desaceleração
	habilitarInterface(false);
}

void QtSerialTest::on_pbFimCursoNegativo_clicked() {
	lbEncoderEmPosicao->setPixmap(QPixmap(":/icones/0"));
	lbPassosEmPosicao->setPixmap(QPixmap(":/icones/0"));
	mDispositivo->buscarFimCurso(IDispositivoMotor::Direita, IDispositivoMotor::Descida);	//	assume o padrão de 6350 na distância de desaceleração
	habilitarInterface(false);
}

void QtSerialTest::on_lePosEncoder_selectionChanged() {
	if ( ! lePosEncoder->isReadOnly()) {
		mEditandoMoverEncoder = true;
	}
	mEditandoMoverPassos = false;
}

void QtSerialTest::on_lePosPassos_selectionChanged() {
	if ( ! lePosPassos->isReadOnly()) {
		mEditandoMoverPassos = true;
	}
	mEditandoMoverEncoder = false;
}

void QtSerialTest::on_rbVelocidadeAlta_toggled(bool checked) {
	if (checked) {
		mDispositivo->usarVelocidadeAlta();
	}
}

void QtSerialTest::on_rbVelocidadeMedia_toggled(bool checked) {
	if (checked) {
		mDispositivo->usarVelocidadeMedia();
	}
}

void QtSerialTest::on_rbVelocidadeBaixa_toggled(bool checked) {
	if (checked) {
		mDispositivo->usarVelocidadeBaixa();
	}
}

void QtSerialTest::on_twSaidas_clicked(QModelIndex index) {
	if ( ! mPortaAcabouAbrirVerificaSaidas) {
		if (index.column() == 1) {
			int linha = index.row();
			QTableWidgetItem * item = twSaidas->item(linha, index.column());
			if ( item->checkState() == Qt::Checked ) {
				item->setCheckState(Qt::Unchecked);
				mDispositivo->ativaSaida(linha, false);
			} else {
				item->setCheckState(Qt::Checked);
				mDispositivo->ativaSaida(linha, true);
			}
		}
	}
}

void QtSerialTest::on_pbTesteVaiVolta_clicked(bool estado) {
	if (estado) {
		mEmTesteVaiVolta = true;
		habilitarInterface(false);
		qint32 temp = leVaiVoltaQtd->text().toInt();
		if ( (temp > 0) && (temp <= 1400000) )	{
			leVaiVoltaQtd->setEnabled(false);
			mDispositivo->iniciarTesteVaiVolta(temp);
		}
	} else {
		mEmTesteVaiVolta = false;
		mDispositivo->finalizarTesteVaiVolta();
		habilitarInterface(true);
		leVaiVoltaQtd->setEnabled(true);
	}
}

void QtSerialTest::slotPosicao_uMAtual(IDispositivoMotor::tipo tipoMotor, qint32 posEnc) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		if ( ! mEditandoMoverEncoder ) {
			lePosEncoder->setText(QString::number(posEnc));
			mPosEncoder = posEnc;
			if ( !mEmMovimento && ( abs( mPosEncoder - mPosEncoderDesejada ) <= ( mDispositivo->pegarDivisorEncoder_uM() / mDispositivo->pegarDividendoEncoder_uM() ) ) && mMovendoEncoder ) {
				mMovendoEncoder = false;
				habilitarInterface(true);
				QApplication::beep();
			}
		}
	}
}

void QtSerialTest::slotPosicaoPassosAtual(IDispositivoMotor::tipo tipoMotor, qint32 posPas) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		mPosPassos = posPas;
		if ( ! mEditandoMoverPassos) {
			lePosPassos->setText(QString::number(mPosPassos));
		}
		if (mPosEncoder != 0) {
			leProporcao->setText(QString::number((double)mPosPassos / (double)mPosEncoder, 'f', 3));
		} else {
			leProporcao->setText(QString::fromUtf8("∞"));
		}
	}
}

void QtSerialTest::slotEstado(IDispositivoMotor::tipo tipoMotor, quint16 estado ) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		int TesteBin = 1;
		int Contagem = 0;
		int LimiteContagem = mDispositivo->quantidadeEstados();
		while (Contagem < LimiteContagem ) {
			if ( ( estado & TesteBin ) > 0 ) {
				twEstados->item(Contagem,1)->setIcon(QIcon(":/icones/1"));
			} else {
				twEstados->item(Contagem,1)->setIcon(QIcon(":/icones/0"));
			}
			TesteBin = TesteBin << 1;
			Contagem++;
		}
	}
}

void QtSerialTest::slotDispositivoEmMovimento(IDispositivoMotor::tipo tipoMotor, bool estado) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		mEmMovimento = estado;
		if ( ( ! mEmMovimento ) && ( ! mMovendoEncoder ) ) {
			habilitarInterface();
		}
		if ( ! mEmMovimento ) {
			if (mPosPassosDesejada == mPosPassos) {
				lbPassosEmPosicao->setPixmap(QPixmap(":/icones/1"));
			}
			if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCilindro) {
				if (1 >= abs( ( ( mPosEncoderDesejada + (qint32)720000 ) % (qint32)360000 ) - ( ( mPosEncoder + (qint32)720000 ) % (qint32)360000 ) )) {
					lbEncoderEmPosicao->setPixmap(QPixmap(":/icones/1"));
					mMovendoEncoder = false;
					habilitarInterface(true);
				}
			} else {
				if (5 >= abs(mPosEncoderDesejada - mPosEncoder)) {
					lbEncoderEmPosicao->setPixmap(QPixmap(":/icones/1"));
					mMovendoEncoder = false;
					habilitarInterface(true);
				}
			}
		}
	}
}

void QtSerialTest::slotAlarme(IDispositivoMotor::tipo tipoMotor, quint16 alarme) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		int TesteBin = 1;
		int Contagem = 0;
		int LimiteContagem = mDispositivo->quantidadeAlarmes();
		while (Contagem < LimiteContagem ) {
			if ( ( alarme & TesteBin ) > 0 ) {
				twAlarmes->item(Contagem,1)->setIcon(QIcon(":/icones/1"));
			} else {
				twAlarmes->item(Contagem,1)->setIcon(QIcon(":/icones/0"));
			}
			TesteBin = TesteBin << 1;
			Contagem++;
		}
	}
}

void QtSerialTest::slotEntrada(IDispositivoMotor::tipo tipoMotor, quint16 entradas) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		int TesteBin = 1;
		int Contagem = 0;
		int LimiteContagem = mDispositivo->quantidadeEntradas();
		while (Contagem < LimiteContagem ) {
			if ( ( entradas & TesteBin ) > 0 ) {
				twEntradas->item(Contagem,0)->setIcon(QIcon(":/icones/1"));
			} else {
				twEntradas->item(Contagem,0)->setIcon(QIcon(":/icones/0"));
			}
			TesteBin = TesteBin << 1;
			Contagem++;
		}
	}
}

void QtSerialTest::slotSaida(IDispositivoMotor::tipo tipoMotor, quint16 saidas) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		int TesteBin = 1;
		int Contagem = 0;
		int LimiteContagem = mDispositivo->quantidadeSaidas();
		while (Contagem < LimiteContagem ) {
			if ( ( saidas & TesteBin ) > 0 ) {
				twSaidas->item(Contagem,0)->setIcon(QIcon(":/icones/1"));
				if (mPortaAcabouAbrirVerificaSaidas) {
					twSaidas->item(Contagem,1)->setCheckState(Qt::Checked);
				}
			} else {
				twSaidas->item(Contagem,0)->setIcon(QIcon(":/icones/0"));
				if (mPortaAcabouAbrirVerificaSaidas) {
					twSaidas->item(Contagem,1)->setCheckState(Qt::Unchecked);
				}
			}
			TesteBin = TesteBin << 1;
			Contagem++;
		}
		mPortaAcabouAbrirVerificaSaidas = false;
	}
}

void QtSerialTest::slotEstadoComunicacao(IDispositivoMotor::tipo tipoMotor, bool estado) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		QString tipo;
		switch (tipoMotor) {
			case IDispositivoMotor::TipoCamera1 :
				tipo = NOME_ACESSO_DISPOSITIVO_CAM1;
				break;
			case IDispositivoMotor::TipoCamera2 :
				tipo = NOME_ACESSO_DISPOSITIVO_CAM2;
				break;
			case IDispositivoMotor::TipoCilindro :
				tipo = NOME_ACESSO_DISPOSITIVO_CILI;
				break;
			case IDispositivoMotor::TipoIndefinido :
				tipo = NOME_ACESSO_DISPOSITIVO_NOVO;
				break;
			default:
				tipo = "desc";
		}
		if ( ! mMensagensAtualizadas ) {
			QString aux1;
			bool aux2;
			for (int i = 0 ; i < mDispositivo->quantidadeEntradas() ; i++) {
				aux2 = mDispositivo->valorEntrada(i, &aux1);
				if (aux2) {
					twEntradas->item(i,0)->setIcon(QIcon(":/icones/1"));
				} else {
					twEntradas->item(i,0)->setIcon(QIcon(":/icones/0"));
				}
				twEntradas->item(i,1)->setText(aux1);
			}
			for (int i = 0; i < mDispositivo->quantidadeSaidas() ; i++) {
				aux2 = mDispositivo->valorSaida(i, &aux1);
				if (aux2) {
					twSaidas->item(i,0)->setIcon(QIcon(":/icones/1"));
					twSaidas->item(i,1)->setCheckState(Qt::Checked);
				} else {
					twSaidas->item(i,0)->setIcon(QIcon(":/icones/0"));
					twSaidas->item(i,1)->setCheckState(Qt::Unchecked);
				}
				twSaidas->item(i,2)->setText(aux1);
			}
			mMensagensAtualizadas = true;
		}

		if (estado) {
			lbComunicacao_1->setPixmap(QPixmap(":/icones/VD"));
			lbComunicacao_2->setText(QString::fromUtf8("comunicação normal, dispositivo %1 - %2").arg(tipo).arg(mDispositivo->modeloDispositivo()));
			if (mPortaAberta) {
				gbEstados->setEnabled(true);
				gbAlarmes->setEnabled(true);
				gbEncoder->setEnabled(true);
				gbPassos->setEnabled(true);
				gbProporcao->setEnabled(true);
				gbJog->setEnabled(true);
				gbEntradas->setEnabled(true);
				gbSaidas->setEnabled(true);
				// gbComandoUsuario->setEnabled(true);
				gbVelocidade->setEnabled(true);
			}
		} else {
			lbComunicacao_1->setPixmap(QPixmap(":/icones/VM"));
			lbComunicacao_2->setText(QString::fromUtf8("FALHA NA COMUNICAÇÃO, DISPOSITIVO %1").arg(tipo));
			if (mPortaAberta) {
				gbEstados->setEnabled(false);
				gbAlarmes->setEnabled(false);
				gbEncoder->setEnabled(false);
				gbPassos->setEnabled(false);
				gbProporcao->setEnabled(false);
				gbJog->setEnabled(false);
				gbEntradas->setEnabled(false);
				gbSaidas->setEnabled(false);
				// gbComandoUsuario->setEnabled(false);
				gbVelocidade->setEnabled(false);
			}
			slotEstado(tipoMotor, 0);
			slotAlarme(tipoMotor, 0);
			slotEntrada(tipoMotor, 0);
			slotSaida(tipoMotor, 0);
		}
	}
}

void QtSerialTest::slotInicializado(IDispositivoMotor::tipo tipoMotor) {
	if (tipoMotor == mTipoDispositivoEmUso) {
/*		if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCamera1) {
			mDispositivo->p_PosicaoReferenciaAbsoluta = 0;	// 283860 para a câmera 2, na travessa de desenvolvimento;
		}
		if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCamera2) {
			mDispositivo->p_PosicaoReferenciaAbsoluta = 283860;	// na travessa de desenvolvimento;
		}
		if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCilindro) {
			mDispositivo->p_PosicaoReferenciaAbsoluta = 0;
		}
		if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoIndefinido) {
			mDispositivo->p_PosicaoReferenciaAbsoluta = 0;
		}
		if (cbPortas->currentIndex() >= 0) {
			cbPortas->setItemText(cbPortas->currentIndex(), mDispositivo->tipoEmNome(mDispositivo->tipoDispositivo()));
			cbPortas->setToolTip(mDispositivo->portasDisponiveis().at(cbPortas->currentIndex()));
		}
		if (mDispositivo->tipoDispositivo() == IDispositivoMotor::TipoCilindro ) {
			mDispositivo->p_CorrenteMovimento = 200;
			mDispositivo->p_CorrenteParado = 180;
			mDispositivo->p_VelocidadeAlta = 2640;
			mDispositivo->p_VelocidadeBaixa = 440;
			mDispositivo->p_VelocidadeMedia = 1760;
			mDispositivo->p_VelocidadeMaxima = 7744;
			mDispositivo->p_VelocidadeMinima = 1;
		} else {
			mDispositivo->p_CorrenteMovimento = 200;
			mDispositivo->p_CorrenteParado = 30;
			mDispositivo->p_VelocidadeAlta = 7040; 	// 3520;	// 1760;
			mDispositivo->p_VelocidadeBaixa = 440;
			mDispositivo->p_VelocidadeMedia = 5280;	//	3520;	// 1760; 	// 880;
			mDispositivo->p_VelocidadeMaxima = 7744;
			mDispositivo->p_VelocidadeMinima = 1;
		}
*/		if (pbAbrirPorta->isChecked()) {
			gbComandoUsuario->setEnabled(true);
			leComando->selectAll();
			leComando->setFocus();
		}
		mPortaAcabouAbrirVerificaSaidas = true;
	}
}

void QtSerialTest::habilitarInterface(bool estado, QPushButton *excessao) {
	if (estado) {
		if ( ! mEmTesteVaiVolta ) {
			pbMoverEncoder->setEnabled(true);
			pbZeraEncoder->setEnabled(true);
			pbMoverPassos->setEnabled(true);
			pbZeraPassos->setEnabled(true);
			pbNegativo->setEnabled(true);
			pbPositivo->setEnabled(true);
			pbFimCursoPositivo->setEnabled(true);
			pbFimCursoNegativo->setEnabled(true);
			gbEstados->setEnabled(true);
			gbAlarmes->setEnabled(true);
			gbEntradas->setEnabled(true);
			gbSaidas->setEnabled(true);
			// gbComandoUsuario->setEnabled(true);
			gbVelocidade->setEnabled(true);
			lePosPassos->setReadOnly(false);
			lePosEncoder->setReadOnly(false);
			leVaiVoltaQtd->setEnabled(true);
			pbTesteVaiVolta->setEnabled(true);
		}
	} else {
		lbPassosEmPosicao->setPixmap(QPixmap(":/icones/0"));
		lbEncoderEmPosicao->setPixmap(QPixmap(":/icones/0"));
		if (excessao != pbMoverEncoder) {
			pbMoverEncoder->setEnabled(false);
		}
		if (excessao != pbZeraEncoder) {
			pbZeraEncoder->setEnabled(false);
		}
		if (excessao != pbMoverPassos) {
			pbMoverPassos->setEnabled(false);
		}
		if (excessao != pbZeraPassos) {
			pbZeraPassos->setEnabled(false);
		}
		if (excessao != pbNegativo) {
			pbNegativo->setEnabled(false);
		}
		if (excessao != pbPositivo) {
			pbPositivo->setEnabled(false);
		}
		if (excessao != pbFimCursoPositivo) {
			pbFimCursoPositivo->setEnabled(false);
		}
		if (excessao != pbFimCursoNegativo) {
			pbFimCursoNegativo->setEnabled(false);
		}
		gbEstados->setEnabled(false);
		gbAlarmes->setEnabled(false);
		gbSaidas->setEnabled(false);
		gbEntradas->setEnabled(false);
		// gbComandoUsuario->setEnabled(false);
		gbVelocidade->setEnabled(false);
		lePosPassos->setReadOnly(true);
		lePosEncoder->setReadOnly(true);
		leVaiVoltaQtd->setEnabled(false);
		if ( ! mEmTesteVaiVolta ) {
			pbTesteVaiVolta->setEnabled(false);
		}
	}
}

void QtSerialTest::slotRespostaAComandoUsuario(IDispositivoMotor::tipo tipoMotor, QString resposta) {
	if (tipoMotor == mTipoDispositivoEmUso) {
		leResposta->setText(resposta.simplified());
	}
}

void QtSerialTest::slotSemRespostaHabilitaInterface() {
	QMessageBox * mb = new QMessageBox(QMessageBox::Critical, "Tempo de espera excedido", QString::fromUtf8("Nenhuma resposta recebida.\nLiberar a interface?"), QMessageBox::Yes | QMessageBox::No, this);
	if ( mb->exec() == QMessageBox::Yes ) {
		habilitarInterface();
	} else {
		close();
	}
}
